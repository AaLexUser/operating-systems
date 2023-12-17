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
#include <linux/vmalloc.h>
#include <linux/tick.h>
#include <linux/jiffies.h>
#include <linux/time64.h>
#include <linux/math64.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksei Lapin");
MODULE_DESCRIPTION("Simple module to start");

struct ioctl_arg{
    u64 val;
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
#define GET_CPU_STAT_BY_NUM _IOR(IOC_MAGIC, 2, struct cpustat)
#define GET_ONLINE_CPU_NUM _IOR(IOC_MAGIC, 3, struct ioctl_arg)
#define GET_POSSIBLE_CPU_NUM _IOR(IOC_MAGIC, 4, struct ioctl_arg)
#define GET_CPU_STAT_ALL _IOR(IOC_MAGIC, 5, struct cpustat*)

#define DEVICE_NAME "os_lab"


#define NSEC_TO_SEC(nsec) (nsec) / 1000000000u
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
static u64 get_possible_cpu_num(void);
static u64 get_online_cpu_num(void);
static struct cpustat* get_cpustat_by_num(u64 cpu_num);
static struct cpustat* get_cpustat_all(void);



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
        case GET_ONLINE_CPU_NUM:{
            cpu_num.val = get_online_cpu_num();
            if(copy_to_user((struct ioctl_arg*) arg, &cpu_num, sizeof(struct ioctl_arg))){
                pr_err("Fail to copy to user space.");
            }
            return 0;
        }
        case GET_POSSIBLE_CPU_NUM: {
            cpu_num.val = get_possible_cpu_num();
            if(copy_to_user((struct ioctl_arg*) arg, &cpu_num, sizeof(struct ioctl_arg))){
                pr_err("Fail to copy to user space.");
            }
            return 0;
        }
        case GET_CPU_STAT_BY_NUM:{
            struct cpustat* stat = NULL;
            stat = get_cpustat_by_num(cpu);
            if(copy_to_user((struct cpustat*) arg, stat, sizeof(struct cpustat))){
                pr_err("Fail to copy to user space.");
            }
            return 0;
        }
        case GET_CPU_STAT_ALL:{
            struct cpustat* stat = NULL;
            u64 buffer_size = (get_possible_cpu_num() + 1) * sizeof(struct cpustat);
            stat = get_cpustat_all();
            if(copy_to_user((struct cpustat*) arg, stat, buffer_size)){
                pr_err("Fail to copy to user space.");
            }
            return 0;
    
        }
        case SET_CPU: {
            if(copy_from_user(&cpu_num, (struct ioctl_arg*) arg, sizeof(struct ioctl_arg))){
                pr_err("Fail to copy to kernel space.");
            }
            cpu = cpu_num.val;
            return 0;
        }
        default:
            pr_err("Unknown command\n");
    }
    return 0;
}

static u64 get_possible_cpu_num(void){
    u64 cpu_num = num_present_cpus();
    return cpu_num;
}

static u64 get_online_cpu_num(void){
    u64 cpu_num = num_online_cpus();
    return cpu_num;
}

u64 cs_nsec_to_clock_t(u64 x)
{
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	/*
         * max relative error 5.7e-8 (1.8s per year) for USER_HZ <= 1024,
         * overflow after 64.99 years.
         * exact for HZ=60, 72, 90, 120, 144, 180, 300, 600, 900, ...
         */
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

#ifdef arch_idle_time
static u64 cs_get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle;

	idle = kcs->cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

static u64 cs_get_iowait_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 iowait;

	iowait = kcs->cpustat[CPUTIME_IOWAIT];
	if (cpu_online(cpu) && nr_iowait_cpu(cpu))
		iowait += arch_idle_time(cpu);
	return iowait;
}

#else

static u64 cs_get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle, idle_usecs = -1ULL;

	if (cpu_online(cpu))
		idle_usecs = get_cpu_idle_time_us(cpu, NULL);

	if (idle_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcs->cpustat[CPUTIME_IDLE];
	else
		idle = idle_usecs * NSEC_PER_USEC;

	return idle;
}

static u64 cs_get_iowait_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 iowait, iowait_usecs = -1ULL;

	if (cpu_online(cpu))
		iowait_usecs = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcs->cpustat[CPUTIME_IOWAIT];
	else
		iowait = iowait_usecs * NSEC_PER_USEC;

	return iowait;
}

#endif 

static struct cpustat* get_cpustat_by_num(u64 cpu_num){
    struct kernel_cpustat *kcs = &kcpustat_cpu(cpu_num);
    struct cpustat* stat = vmalloc(sizeof(struct cpustat));
    if (!stat) {
        pr_err("vmalloc failed.\n");
        return NULL;
    }   
    stat->user = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_USER]);
    stat->nice = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_NICE]);
    stat->system = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SYSTEM]);
    stat->idle = cs_nsec_to_clock_t(cs_get_idle_time(kcs, cpu_num));
    stat->iowait = cs_nsec_to_clock_t(cs_get_iowait_time(kcs, cpu_num));
    stat->irq = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_IRQ]);
    stat->softirq = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SOFTIRQ]);
    stat->steal = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_STEAL]);
    stat->guest = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST]);
    stat->guest_nice = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST_NICE]);
    return stat;
}

static struct cpustat* get_cpustat_all(void){
    int i = 0;
    u64 cpu_num = get_online_cpu_num() + 1;
    struct cpustat* stat_array = vmalloc(cpu_num * sizeof(struct cpustat));
    if (!stat_array) {
        pr_err("vmalloc failed.\n");
        return NULL;
    } 
    for_each_possible_cpu(i) {
		struct kernel_cpustat *kcs = &kcpustat_cpu(i);
		stat_array[0].user += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_USER]);
		stat_array[0].nice += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_NICE]);
		stat_array[0].system += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SYSTEM]);
		stat_array[0].idle += cs_nsec_to_clock_t(cs_get_idle_time(kcs, cpu_num));
		stat_array[0].iowait += cs_nsec_to_clock_t(cs_get_iowait_time(kcs, cpu_num));
		stat_array[0].irq += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_IRQ]);
		stat_array[0].softirq += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SOFTIRQ]);
		stat_array[0].steal += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_STEAL]);
		stat_array[0].guest += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST]);
		stat_array[0].guest_nice += cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST_NICE]);
    }
    
    for_each_online_cpu(i) {

        struct kernel_cpustat *kcs = &kcpustat_cpu(i);
        stat_array[i+1].user = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_USER]);
        stat_array[i+1].nice = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_NICE]);
        stat_array[i+1].system = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SYSTEM]);
        stat_array[i+1].idle = cs_nsec_to_clock_t(cs_get_idle_time(kcs, i));
        stat_array[i+1].iowait = cs_nsec_to_clock_t(cs_get_iowait_time(kcs, i));
        stat_array[i+1].irq = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_IRQ]);
        stat_array[i+1].softirq = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_SOFTIRQ]);
        stat_array[i+1].steal = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_STEAL]);
        stat_array[i+1].guest = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST]);
        stat_array[i+1].guest_nice = cs_nsec_to_clock_t(kcs->cpustat[CPUTIME_GUEST_NICE]);
    }
    return stat_array;
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

