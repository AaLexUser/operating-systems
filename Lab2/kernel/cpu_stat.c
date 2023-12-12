#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksei Lapin");
MODULE_DESCRIPTION("Simple module to start");

struct ioctl_arg{
    unsigned int val;
};

/* Documentation/ioctl/ioctl-number.txt */
#define IOC_MAGIC 'a'

#define WR_VALUE _IOW(IOC_MAGIC, 0, struct ioctl_arg)

#define DEVICE_NAME "os_lab"

static dev_t major = 0; /* The major number assigned to the device driver */
static struct class *dev_class;
static struct cdev cs_dev;

static int __init cpu_stat_init(void);
static void __exit cpu_stat_exit(void);
static ssize_t cs_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t cs_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int cs_open(struct inode *inode, struct file *file);
static int cs_release(struct inode *inode, struct file *file);

static struct file_operations fops = {
    .owner      = THIS_MODULE,
    .read       = cs_read,
    .write      = cs_write,
    .open       = cs_open,
    .release    = cs_release,
};

static ssize_t cs_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
    pr_info("Reading from device.\n");
    return 0;
}

static ssize_t cs_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
    pr_info("Writing to device.\n");
    return 0;
}

static int cs_open(struct inode *inode, struct file *file){
    pr_info("Device opened.\n");
    return 0;
}

static int cs_release(struct inode *inode, struct file *file){
    pr_info("Device closed.\n");
    return 0;
}


static int __init cpu_stat_init(void){
    pr_info("cpu_stat: Module loaded\n");

    /* Allocating major numbers */
    if(alloc_chrdev_region(&major, 0, 1, DEVICE_NAME) < 0){
        pr_err("Cannot allocate major numbers.\n");
        return -1;
    }

    /* cdev structure initialization */
    cdev_init(&cs_dev, &fops);

    /* Adding device to the system */
    if(cdev_add(&cs_dev, major, 1) < 0){
        pr_err("Cannot add the device to the system.\n");
        goto rm_major;
    }

    /* Creating structure class */
    if((dev_class = class_create(THIS_MODULE, DEVICE_NAME)) == NULL) {
        pr_err("Cannot create the structure class.\n");
        goto rm_major;
    }

    if(device_create(dev_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME) < 0){
        pr_err("Cannot create the device");
        goto rm_class;
    }

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return 0;

rm_class:
    class_destroy(dev_class);
rm_major:
    unregister_chrdev_region(major, 1);

    return -1;
}

static void __exit cpu_stat_exit(void){
    device_destroy(dev_class, MKDEV(major, 0));
    class_destroy(dev_class);
    cdev_del(&cs_dev);
    unregister_chrdev_region(major, 1);
    pr_info("cpu_stat: Module unloaded.\n");
}

module_init(cpu_stat_init);
module_exit(cpu_stat_exit);

