#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <linux/list.h>

#include "char_driver.h"
#include "urb_serializer.h"


#include <linux/file.h>
#include <linux/net.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

/* Hardening for Spectre-v1 */
#include <linux/nospec.h>

#include "usbip_common.h"
#include "vhci.h"

#define VUSB_MAJOR      42
#define VUSB_MAX_MINOR  5

#define KBUFF_SIZE 4096

#define ATTACH_DEV      1
#define DETACH_DEV      2

struct dev_context {
    struct cdev cdev;
    // ... context
    size_t kbuff_size;
    char *kbuff;
    //struct urb *urb;
    //struct usb_hcd *hcd;
    //int request_len;
};

struct list_item {
    struct list_head list;
    struct urb *urb;
    struct usb_hcd *hcd;
};

static struct list_head urb_reqs;
static DEFINE_SPINLOCK(lock_urb_list);
struct dev_context devs[VUSB_MAX_MINOR];


static int vusb_open(struct inode *inode, struct file *file) {
    struct dev_context *cnxt;

    pr_debug("%s: %s", KBUILD_MODNAME, __func__);

    cnxt = container_of(inode->i_cdev, struct dev_context, cdev);

    file->private_data = cnxt;

    return 0;
}

static ssize_t vusb_read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
    
    /*struct dev_context *cnxt = file->private_data;

    ssize_t len = min(cnxt->kbuff_size, size);

    pr_debug("%s: %s devs[i].kbuff=%p", KBUILD_MODNAME, __func__, cnxt->kbuff);

    pr_info("%s: %s about to read %ld bytes request_len=%d\n",
		KBUILD_MODNAME, __func__, size, cnxt->request_len);

    if(len <= 0 || !cnxt->request_len) {
        return 0;
    }*/

    unsigned long	flags;
    int empty;
    ssize_t ret = 0;

    //pr_info("%s: %s Here 1 \n", KBUILD_MODNAME, __func__);

    spin_lock_irqsave(&lock_urb_list, flags);
    
    empty = list_empty(&urb_reqs);

    //pr_info("%s: %s Here 2 \n", KBUILD_MODNAME, __func__);

    
    if(!empty) {

        pr_info("%s: %s Here 3 \n", KBUILD_MODNAME, __func__);


        struct list_item *entry=list_entry(urb_reqs.next, struct list_item, list);
        
        pr_info("%s: %s Here 4 \n", KBUILD_MODNAME, __func__);

        if(copy_to_user(buffer, entry->urb->setup_packet, 8)) {
            ret = -EFAULT;
        }

        pr_info("%s: %s Here 5 \n", KBUILD_MODNAME, __func__);

        ret = 8;
    }
    //pr_info("%s: %s Here 6 \n", KBUILD_MODNAME, __func__);

    spin_unlock_irqrestore(&lock_urb_list, flags); 

    //pr_info("%s: %s Here 7 \n", KBUILD_MODNAME, __func__);

	return ret;
}

static  ssize_t vusb_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
    
    struct dev_context *cnxt = file->private_data;
    
    ssize_t len = min(cnxt->kbuff_size, size);

    if(len <= 0) {
        return 0;
    }

	pr_info("%s: %s Here 1 \n", KBUILD_MODNAME, __func__);
	
    if(copy_from_user(cnxt->kbuff, buffer, len)) {
        return -EFAULT;
    }

    //print_hex_dump(KERN_DEBUG, "cnxt->kbuff ", DUMP_PREFIX_NONE, 0,  
    //            1, cnxt->kbuff, 20, false);

    unsigned long	flags;

    pr_info("%s: %s Here 2 \n", KBUILD_MODNAME, __func__);

    spin_lock_irqsave(&lock_urb_list, flags);

    pr_info("%s: %s Here 3 \n", KBUILD_MODNAME, __func__);

    struct list_item *entry=list_entry(urb_reqs.next, struct list_item, list);
    pr_info("%s: %s Here 4 \n", KBUILD_MODNAME, __func__);
    list_del(&entry->list);
    pr_info("%s: %s Here 5 \n", KBUILD_MODNAME, __func__);

    spin_unlock_irqrestore(&lock_urb_list, flags); 

    ssize_t kbuff_offset = 0;
    pr_info("%s: %s Here 6 \n", KBUILD_MODNAME, __func__);

    memcpy(&entry->urb->status, cnxt->kbuff + kbuff_offset, sizeof(entry->urb->status));
    kbuff_offset += sizeof(entry->urb->status);

    memcpy(&entry->urb->actual_length, cnxt->kbuff + kbuff_offset, sizeof(entry->urb->actual_length));
    kbuff_offset += sizeof(entry->urb->actual_length);

    memcpy(entry->urb->transfer_buffer, cnxt->kbuff + kbuff_offset, entry->urb->actual_length);
    kbuff_offset += sizeof(entry->urb->transfer_buffer);

    pr_info("%s: %s Here 7 \n", KBUILD_MODNAME, __func__);
    
    urb2log(entry->urb, "User response");

   	usb_hcd_unlink_urb_from_ep(entry->hcd, entry->urb);
    usb_hcd_giveback_urb(entry->hcd, entry->urb, entry->urb->status);

    pr_info("%s: %s Here 8 \n", KBUILD_MODNAME, __func__);
    kfree(entry);

	return kbuff_offset;
}

static  long vusb_ioctl(struct file *file, unsigned int cmd, unsigned long args) {
    
    //struct dev_context *cnxt = file->private_data;

    pr_info("%s: %s recive new cmd %d\n",
		KBUILD_MODNAME, __func__, cmd);

    if (cmd == 0) {

        struct usb_hcd *hcd;
        struct vhci_hcd *vhci_hcd;
        struct vhci_device *vdev;
        struct vhci *vhci;
        __u32 port = 0, pdev_nr = 0, rhport = 0, devid = 0, speed = USB_SPEED_HIGH;

        pdev_nr = port_to_pdev_nr(port);
	    rhport = port_to_rhport(port);

        hcd = platform_get_drvdata(vhcis[pdev_nr].pdev);
        if (hcd == NULL) {
            //dev_err(dev, "port %d is not ready\n", port);
            pr_info("%s: %s port %d is not ready\n",
		        KBUILD_MODNAME, __func__, port);
            return -EAGAIN;
        }

        vhci_hcd = hcd_to_vhci_hcd(hcd);
	    vhci = vhci_hcd->vhci;

        vdev = &vhci->vhci_hcd_hs->vdev[rhport];
        
        vdev->devid         = devid;
        vdev->speed         = speed;
        vdev->ud.status     = VDEV_ST_NOTASSIGNED;
        usbip_kcov_handle_init(&vdev->ud);

        rh_port_connect(vdev, speed);

        pr_info("Device attached\n");
        
        return 0;
    }
    return -EINVAL;
}

static int vusb_release(struct inode *inode, struct file *file) {
    pr_debug("%s: %s", KBUILD_MODNAME, __func__);
    return 0;
}

const struct file_operations vusb_fops = {
    .owner                  = THIS_MODULE,
    .open                   = vusb_open,
    .release                = vusb_release,
    .read                   = vusb_read,
    .write                  = vusb_write,
    .unlocked_ioctl         = vusb_ioctl
};

void RequestUrb(struct urb *urb, struct usb_hcd *hcd) {
    
    struct list_item *it = kmalloc(sizeof(struct list_item), GFP_KERNEL);
    
    it->urb = urb;
    it->hcd = hcd;

    pr_info("%s: %s Here 1 \n", KBUILD_MODNAME, __func__);

    unsigned long	flags;
    spin_lock_irqsave(&lock_urb_list, flags);

    //pr_info("%s: %s Here 2 \n", KBUILD_MODNAME, __func__);

    list_add_tail(&it->list, &urb_reqs);

    //pr_info("%s: %s Here 3 \n", KBUILD_MODNAME, __func__);

    spin_unlock_irqrestore(&lock_urb_list, flags);

    pr_info("%s: %s Here 4 \n", KBUILD_MODNAME, __func__);

    /*devs[0].urb = urb;
    devs[0].hcd = hcd;
    
    ssize_t offset = 0;

    memcpy(devs[0].kbuff, urb->setup_packet, 8);
    offset += 8;

    print_hex_dump(KERN_DEBUG, "devs[0].kbuff ", DUMP_PREFIX_NONE, 0,  
                    1, devs[0].kbuff, 20, false);

    devs[0].request_len = offset;*/
}

int vusb_init(void) {

    pr_debug("%s: %s", KBUILD_MODNAME, __func__);

    int err = register_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR, "vusb");

    if(err != 0){
        return err;
    }

    INIT_LIST_HEAD(&urb_reqs);

    int i;

    for(i = 0; i < VUSB_MAX_MINOR; i++) {
        
        devs[i].kbuff_size = 0;

        devs[i].kbuff = kmalloc(KBUFF_SIZE, GFP_KERNEL);
        if(devs[i].kbuff == NULL)
            goto no_mem;

        pr_debug("%s: %s devs[%d].kbuff=%p", KBUILD_MODNAME, __func__, i, devs[i].kbuff);

        devs[i].kbuff_size = KBUFF_SIZE;
        //devs[i].request_len = 0;

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

