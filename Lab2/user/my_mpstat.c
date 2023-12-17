#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#define IOC_MAGIC 'a'

#define WR_VALUE _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define SET_CPU _IOW(IOC_MAGIC, 1, struct ioctl_arg)
#define GET_CPU_STAT_BY_NUM _IOR(IOC_MAGIC, 2, struct cpustat)
#define GET_ONLINE_CPU_NUM _IOR(IOC_MAGIC, 3, struct ioctl_arg)
#define GET_POSSIBLE_CPU_NUM _IOR(IOC_MAGIC, 4, struct ioctl_arg)
#define GET_CPU_STAT_ALL _IOR(IOC_MAGIC, 5, struct cpustat*)


#define DEVICE_PATH "/dev/os_lab"

struct ioctl_arg{
    uint64_t val;
};

struct cpustat {
    uint64_t user;
    uint64_t nice;
    uint64_t system;
    uint64_t idle;
    uint64_t iowait;
    uint64_t irq;
    uint64_t softirq;
    uint64_t steal;
    uint64_t guest;
    uint64_t guest_nice;
};

uint64_t possible_cpu = 0;

int main(int argc, char *argv[]){
    int driver = open(DEVICE_PATH, O_RDWR);
    if(driver < 0){
        printf("Fail to open device.\n");
        return -1;
    }
    struct ioctl_arg umsg;
    ioctl(driver, GET_POSSIBLE_CPU_NUM, &umsg);
    uint64_t possible_cpu = umsg.val;
    struct cpustat* cpus_stat = malloc((possible_cpu + 1) * sizeof(struct cpustat));
    if(cpus_stat == NULL){
        printf("CPUS_STAT is NULL");
        return -1;
    }
    ioctl(driver, GET_CPU_STAT_ALL, cpus_stat);
    printf("Total user stat: %"PRIu64"\n", cpus_stat[0].user);
    close(driver);
    return 0;
}