#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "char_driver.h"
#include "urb_serializer.h"

#define VUSB_MAJOR      42
#define VUSB_MAX_MINOR  5

#define KBUFF_SIZE 4096

struct dev_context {
    struct cdev cdev;
    // ... context
    size_t kbuff_size;
    char *kbuff;
    struct urb *urb;
    struct usb_hcd *hcd;
    int request_len;
};

struct dev_context devs[VUSB_MAX_MINOR];

static int vusb_open(struct inode *inode, struct file *file) {
    struct dev_context *cnxt;

    pr_debug("%s: %s", KBUILD_MODNAME, __func__);

    cnxt = container_of(inode->i_cdev, struct dev_context, cdev);

    file->private_data = cnxt;

    return 0;
}

static ssize_t vusb_read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
    
    struct dev_context *cnxt = file->private_data;

    ssize_t len = min(cnxt->kbuff_size, size);

    pr_debug("%s: %s devs[i].kbuff=%p", KBUILD_MODNAME, __func__, cnxt->kbuff);

    pr_info("%s: %s about to read %ld bytes request_len=%d\n",
		KBUILD_MODNAME, __func__, size, cnxt->request_len);

    if(len <= 0 || !cnxt->request_len) {
        return 0;
    }
	
    if(copy_to_user(buffer, cnxt->kbuff, len)){
        return -EFAULT;
    }
	
    cnxt->request_len -= len;

	return len;
}

static  ssize_t vusb_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
    
    struct dev_context *cnxt = file->private_data;
    
    ssize_t len = min(cnxt->kbuff_size, size);

    if(len <= 0) {
        return 0;
    }

	pr_info("%s: %s about to write %ld bytes\n",
		KBUILD_MODNAME, __func__, size);
	
    if(copy_from_user(cnxt->kbuff, buffer, len)) {
        return -EFAULT;
    }

    print_hex_dump(KERN_DEBUG, "cnxt->kbuff ", DUMP_PREFIX_NONE, 0,  
                1, cnxt->kbuff, 20, false);

    ssize_t kbuff_offset = 0;

    memcpy(&cnxt->urb->status, cnxt->kbuff + kbuff_offset, sizeof(cnxt->urb->status));
    kbuff_offset += sizeof(cnxt->urb->status);

    memcpy(&cnxt->urb->actual_length, cnxt->kbuff + kbuff_offset, sizeof(cnxt->urb->actual_length));
    kbuff_offset += sizeof(cnxt->urb->actual_length);

    memcpy(cnxt->urb->transfer_buffer, cnxt->kbuff + kbuff_offset, cnxt->urb->actual_length);
    kbuff_offset += cnxt->urb->actual_length;

    urb2log(cnxt->urb, "User response");

   	usb_hcd_unlink_urb_from_ep(cnxt->hcd, cnxt->urb);
	usb_hcd_giveback_urb(cnxt->hcd, cnxt->urb, cnxt->urb->status);
	
	return len;
}

static int vusb_release(struct inode *inode, struct file *file) {
    pr_debug("%s: %s", KBUILD_MODNAME, __func__);
    return 0;
}

const struct file_operations vusb_fops = {
    .owner = THIS_MODULE,
    .open = vusb_open,
    .release = vusb_release,
    .read = vusb_read,
    .write = vusb_write
};

void RequestUrb(struct urb *urb, struct usb_hcd *hcd) {

    devs[0].urb = urb;
    devs[0].hcd = hcd;
    
    ssize_t offset = 0;

    memcpy(devs[0].kbuff, urb->setup_packet, 8);
    offset += 8;

    print_hex_dump(KERN_DEBUG, "devs[0].kbuff ", DUMP_PREFIX_NONE, 0,  
                    1, devs[0].kbuff, 20, false);

    devs[0].request_len = offset;
}

int vusb_init(void) {

    pr_debug("%s: %s", KBUILD_MODNAME, __func__);

    int err = register_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR, "vusb");

    if(err != 0){
        return err;
    }

    int i;

    for(i = 0; i < VUSB_MAX_MINOR; i++) {
        
        devs[i].kbuff_size = 0;

        devs[i].kbuff = kmalloc(KBUFF_SIZE, GFP_KERNEL);
        if(devs[i].kbuff == NULL)
            goto no_mem;

        pr_debug("%s: %s devs[%d].kbuff=%p", KBUILD_MODNAME, __func__, i, devs[i].kbuff);

        devs[i].kbuff_size = KBUFF_SIZE;
        devs[i].request_len = 0;

        cdev_init(&devs[i].cdev, &vusb_fops);
        cdev_add(&devs[i].cdev, MKDEV(VUSB_MAJOR, i), 1);
    }

    return 0;

    no_mem:

    for (i = 0; i < VUSB_MAX_MINOR; i++) {

        if(devs[i].kbuff_size == 0)
            break;

        kfree(devs[i].kbuff);   
        cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR);
    
    return -ENOMEM;
}

void vusb_cleanup(void) {
    
    int i;

    pr_debug("%s: %s", KBUILD_MODNAME, __func__);

    for(i = 0; i < VUSB_MAX_MINOR; i++) {
        kfree(devs[i].kbuff);
        cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR);
}

