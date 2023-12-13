#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/kernel_stat.h>
#include <linux/types.h> /* u64 */
#include <linux/cpumask.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksei Lapin");
MODULE_DESCRIPTION("Simple module to start");

struct ioctl_arg{
    unsigned int val;
};

static struct cpustat {
    u64 user;
    u64 nice;
    u64 system;
    u64 idle;
    u64 iowait;
    u64 irq;
    u64 softirq;
    u64 steal;
    u64 guest;
    u64 guest_nice;
};

/* Documentation/ioctl/ioctl-number.txt */
#define IOC_MAGIC 'a'

#define WR_VALUE _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define SET_CPU _IOW(IOC_MAGIC, 1, struct ioctl_arg)
#define GET_CPU_STAT _IOR(IOC_MAGIC, 2, struct cpustat)
#define GET_CPU_NUM _IOR(IOC_MAGIC, 3, struct ioctl_arg)

#define DEVICE_NAME "os_lab"

static dev_t major = 0; /* The major number assigned to the device driver */
static struct class *dev_class;
static struct cdev cs_dev;

static u64 cpu = 0;

static int __init cpu_stat_init(void);
static void __exit cpu_stat_exit(void);
static ssize_t cs_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t cs_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int cs_open(struct inode *inode, struct file *file);
static int cs_release(struct inode *inode, struct file *file);
static long cs_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static u64 get_cpu_num(void);
static int get_stat(void);



static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .read           = cs_read,
    .write          = cs_write,
    .open           = cs_open,
    .release        = cs_release,
    .unlocked_ioctl = cs_ioctl,
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

static long cs_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    struct ioctl_arg cpu_num;
    switch (cmd) {
        case GET_CPU_NUM:{
            cpu_num.val = get_cpu_num();
            if(copy_to_user((struct ioctl_arg*) arg, &cpu_num, sizeof(struct ioctl_arg))){
                pr_err("Fail to copy to user space.");
            }
            return 0;
        }
        case GET_CPU_STAT:{
            struct cpustat* stat;
            stat = get_stat(cpu);
            if(copy_to_user((struct cpustat*) arg, &stat, sizeof(struct stat))){
                pr_err("Fail to copy to user space.");
            }
            return 0;
        }
        case SET_CPU: {
            if(copy_from_user(&cpu_num, (struct ioctl_arg*) arg, sizeof(struct ioctl_arg))){
                pr_err("Fail to copy to kernel space.");
            }
            return 0;
        }
        default:
            pr_err("Unknown command\n");
    }
    return 0;
}

static u64 get_cpu_num(void){
    u64 cpu_num = num_online_cpus();
    return cpu_num;
}

static struct cpustat get_cpustat(u64 cpu_num){
    struct kernel_cpustat *kcs = &kcpustat_cpu(i);
    struct cpustat stat= vmalloc(sizeof(struct cpustat));
    stat.user = kcs->cpustat[CPUTIME_USER];
    stat.nice = kcs->cpustat[CPUTIME_NICE];
    stat.system = kcs->cpustat[CPUTIME_SYSTEM];
    stat.idle = get_idle_time(kcs, i);
    stat.iowait = get_iowait_time(kcs, i);
    stat.irq = kcs->cpustat[CPUTIME_IRQ];
    stat.softirq = kcs->cpustat[CPUTIME_SOFTIRQ];
    stat.steal = kcs->cpustat[CPUTIME_STEAL];
    stat.guest = kcs->cpustat[CPUTIME_GUEST];
    stat.guest_nice = kcs->cpustat[CPUTIME_GUEST_NICE];
    return stat;
}

static int get_stat(int cpu_num){
    int i = 0;

    u64 user, nice, system, idle, iowait, irq, softirq, steal;
    u64 guest, guest_nice;

    for_each_online_cpu(cpu) {

        struct kernel_cpustat *kcs = &kcpustat_cpu(i);
        user = kcs->cpustat[CPUTIME_USER];
        nice = kcs->cpustat[CPUTIME_NICE];
        system = kcs->cpustat[CPUTIME_SYSTEM];
        idle = get_idle_time(kcs, i);
        iowait = get_iowait_time(kcs, i);
        irq = kcs->cpustat[CPUTIME_IRQ];
        softirq = kcs->cpustat[CPUTIME_SOFTIRQ];
        steal = kcs->cpustat[CPUTIME_STEAL];
        guest = kcs->cpustat[CPUTIME_GUEST];
        guest_nice = kcs->cpustat[CPUTIME_GUEST_NICE];

    }
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

    if(device_create(dev_class, NULL, major, NULL, DEVICE_NAME) < 0){
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
    device_destroy(dev_class, major);
    class_destroy(dev_class);
    cdev_del(&cs_dev);
    unregister_chrdev_region(major, 1);
    pr_info("cpu_stat: Module unloaded.\n");
}

module_init(cpu_stat_init);
module_exit(cpu_stat_exit);

