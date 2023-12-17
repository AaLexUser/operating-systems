#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

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

struct cpu_percent_stat {
    double user;
    double nice;
    double system;
    double idle;
    double iowait;
    double irq;
    double softirq;
    double steal;
    double guest;
    double guest_nice;
};


uint64_t possible_cpu = 0;


unsigned long long get_global_cpu_mpstats(struct cpustat* cpus_stat, uint64_t possible_cpus){
    unsigned long long tot_jiffies = 0;
    tot_jiffies = cpus_stat[0].user + cpus_stat[0].nice + cpus_stat[0].system 
        + cpus_stat[0].idle + cpus_stat[0].iowait + cpus_stat[0].irq + cpus_stat[0].softirq 
        + cpus_stat[0].steal;
    return tot_jiffies;

}

struct cpu_percent_stat* get_percentage_stat(struct cpustat* cpus_stat, uint64_t possible_cpus, unsigned long long tot_jiffies){
    struct cpu_percent_stat* new_stat = malloc((possible_cpus + 1) * sizeof(struct cpu_percent_stat));
    if(!new_stat){
        printf("New cpu stat is NULL");
        return NULL;
    }
    for(int i = 0; i < possible_cpus + 1; i++){
        new_stat[i].user = (double)(cpus_stat[i].user * 100) / tot_jiffies;
        new_stat[i].system = (double)(cpus_stat[i].system * 100) / tot_jiffies;
        new_stat[i].nice = (double)(cpus_stat[i].nice * 100) / tot_jiffies;
        new_stat[i].idle = (double)(cpus_stat[i].idle * 100) / tot_jiffies;
        new_stat[i].iowait = (double)(cpus_stat[i].iowait * 100) / tot_jiffies;
        new_stat[i].irq = (double)(cpus_stat[i].irq * 100) / tot_jiffies;
        new_stat[i].softirq = (double)(cpus_stat[i].softirq * 100) / tot_jiffies;
        new_stat[i].steal = (double)(cpus_stat[i].steal * 100) / tot_jiffies;
        new_stat[i].guest = (double)(cpus_stat[i].guest * 100) / tot_jiffies;
        new_stat[i].guest_nice = (double)(cpus_stat[i].guest_nice * 100) / tot_jiffies;
    }
    return new_stat;
}

void get_current_time(char* time_str){
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void write_cpu_percent_stat(struct cpu_percent_stat* cpus_stat, uint64_t possible_cpus){
    char time_str[10];
    get_current_time(time_str);
    printf("%s\t", time_str);

    printf("CPU\t%%usr\t%%nice\t%%sys\t%%iowait\t%%irq "
		       "\t%%soft\t%%steal\t%%guest\t%%gnice\t%%idle\n");
    for(int i = 0; i < possible_cpus + 1; i++){
        get_current_time(time_str);
        printf("%s\t", time_str);

        if(i == 0){
            printf("all\t");
        }
        else printf("%d\t", (i - 1));

        printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f"
        "    %.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
            cpus_stat[i].user,
            cpus_stat[i].nice,
            cpus_stat[i].system,
            cpus_stat[i].iowait,
            cpus_stat[i].irq,
            cpus_stat[i].softirq,
            cpus_stat[i].steal,
            cpus_stat[i].guest,
            cpus_stat[i].guest_nice,
            cpus_stat[i].idle
        );
    }
}


void write_cpu_stat(struct cpustat* cpus_stat, uint64_t possible_cpus){
    char time_str[10];
    get_current_time(time_str);
    printf("%s\t", time_str);

    
    
    printf("CPU\t%%usr   %%nice    %%sys %%iowait  %%irq "
		       "%%soft  %%steal  %%guest  %%gnice   %%idle\n");
    for(int i = 0; i < possible_cpus + 1; i++){
        get_current_time(time_str);
        printf("%s\t", time_str);
        if(i == 0){
            printf("all\t");
        }
        else printf("%d\t", (i - 1));

        printf("%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64""
        "    %"PRIu64"\t\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\n",
            cpus_stat[i].user,
            cpus_stat[i].nice,
            cpus_stat[i].system,
            cpus_stat[i].iowait,
            cpus_stat[i].irq,
            cpus_stat[i].softirq,
            cpus_stat[i].steal,
            cpus_stat[i].guest,
            cpus_stat[i].guest_nice,
            cpus_stat[i].idle
        );
    }
}


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
    unsigned long long tot_jiffies = get_global_cpu_mpstats(cpus_stat, possible_cpu);
    printf("global tot_jiffies %llu\n", tot_jiffies);
    struct cpu_percent_stat* cpus_percent_stat = get_percentage_stat(cpus_stat, possible_cpu, tot_jiffies);
    write_cpu_percent_stat(cpus_percent_stat, possible_cpu);
    close(driver);
    return 0;
}