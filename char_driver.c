#include <linux/fs.h>
#include <linux/cdev.h>

#define VUSB_MAJOR      42
#define VUSB_MAX_MINOR  5

struct dev_context {
    struct cdev cdev;
    // ... context
};

struct dev_context devs[VUSB_MAX_MINOR];

static int vusb_open(struct inode *inode, struct file *file) {
    struct dev_context *cnxt;

    pr_debug("hello from %s", __func__);

    cnxt = container_of(inode->i_cdev, struct dev_context, cdev);

    file->private_data = cnxt;

    return 0;
}

static ssize_t vusb_read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
    struct dev_context *cnxt;

    cnxt = file->private_data;

    return 0;
}

static  ssize_t vusb_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
    struct dev_context *cnxt;

    cnxt = file->private_data;

    return 0;
}

static int vusb_release(struct inode *inode, struct file *file) {
    pr_debug("hello from %s", __func__);
    return 0;
}

const struct file_operations vusb_fops = {
    .owner = THIS_MODULE,
    .open = vusb_open,
    .release = vusb_release,
    .read = vusb_read,
    .write = vusb_write
};


int vusb_init(void) {

    pr_debug("hello from vusb_init");

    int err = register_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR, "vusb");

    if(err != 0){
        return err;
    }

    int i;

    for(i = 0; i < VUSB_MAX_MINOR; i++) {
        cdev_init(&devs[i].cdev, &vusb_fops);
        cdev_add(&devs[i].cdev, MKDEV(VUSB_MAJOR, i), 1);
    }

    return 0;
}

void vusb_cleanup(void) {
    int i;

    for(i = 0; i < VUSB_MAX_MINOR; i++) {
        cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(VUSB_MAJOR, 0), VUSB_MAX_MINOR);
}


