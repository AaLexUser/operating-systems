#include "common.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define IOC_MAGIC 'a'

#define GET_OS_STAT _IOR(IOC_MAGIC, 0, struct os_stat)
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


#define __NEW_UTS_LEN 64

struct os_stat{
    char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};



uint64_t possible_cpu = 0;
uint8_t* cpu_mask = NULL;

#define KEY_all "all"


void usage(char *progname)
{
	fprintf(stderr, "Usage: %s [ options ] [ <interval> [ <count> ] ]\n",
		progname);

	fprintf(stderr, "Options are:\n"
			  "[ -P { <cpu_list> | ALL } ]\n");
	exit(1);
}


unsigned long long* get_global_cpu_mpstats(struct cpustat* cpus_stat, uint64_t possible_cpus){
    unsigned long long* tot_jiffies = malloc(sizeof(unsigned long long) * (possible_cpus + 1));
    if(!tot_jiffies){
        printf("Unable to malloc tot_jiffies.");
        return NULL;
    }
    for (int i = 0; i < possible_cpus + 1; i++){
        tot_jiffies[i] = cpus_stat[i].user + cpus_stat[i].nice + cpus_stat[i].system 
        + cpus_stat[i].idle + cpus_stat[i].iowait + cpus_stat[i].irq + cpus_stat[i].softirq 
        + cpus_stat[i].steal + cpus_stat[i].guest + cpus_stat[i].guest_nice;
    }
    return tot_jiffies;

}

struct cpu_percent_stat* get_percentage_stat(struct cpustat* cpus_stat, uint64_t possible_cpus, unsigned long long* tot_jiffies){
    struct cpu_percent_stat* new_stat = malloc((possible_cpus + 1) * sizeof(struct cpu_percent_stat));
    if(!new_stat){
        printf("New cpu stat is NULL");
        return NULL;
    }
    for(int i = 0; i < possible_cpus + 1; i++){
        new_stat[i].user = (double)(cpus_stat[i].user * 100) / tot_jiffies[i];
        new_stat[i].system = (double)(cpus_stat[i].system * 100) / tot_jiffies[i];
        new_stat[i].nice = (double)(cpus_stat[i].nice * 100) / tot_jiffies[i];
        new_stat[i].idle = (double)(cpus_stat[i].idle * 100) / tot_jiffies[i];
        new_stat[i].iowait = (double)(cpus_stat[i].iowait * 100) / tot_jiffies[i];
        new_stat[i].irq = (double)(cpus_stat[i].irq * 100) / tot_jiffies[i];
        new_stat[i].softirq = (double)(cpus_stat[i].softirq * 100) / tot_jiffies[i];
        new_stat[i].steal = (double)(cpus_stat[i].steal * 100) / tot_jiffies[i];
        new_stat[i].guest = (double)(cpus_stat[i].guest * 100) / tot_jiffies[i];
        new_stat[i].guest_nice = (double)(cpus_stat[i].guest_nice * 100) / tot_jiffies[i];
    }
    return new_stat;
}


void write_os_stat(struct os_stat* os_stat){
    printf("OS Information:\n");
    printf("sysname: %s\n", os_stat->sysname);
    printf("nodename: %s\n", os_stat->nodename);
    printf("release: %s\n", os_stat->release);
    printf("version: %s\n", os_stat->version);
    printf("machine: %s\n", os_stat->machine);
    printf("domainname: %s\n", os_stat->domainname);
};

void write_header(struct os_stat* os_stat, uint64_t possible_cpus){
    char date_str[10];
    get_current_date(date_str);
    printf("%s %s (%s)  %s      _%s_        (%"PRIu64" CPU)\n", os_stat->sysname, os_stat->release, os_stat->nodename, date_str,  os_stat->machine, possible_cpus);
    printf("\n");
}

void write_cpu_percent_stat(struct cpu_percent_stat* cpus_stat, uint64_t possible_cpus){
    char time_str[10];
    get_current_time(time_str);
    printf("%s\t", time_str);

    printf("CPU\t%%usr\t%%nice\t%%sys\t%%iowait\t%%irq "
		       "\t%%soft\t%%steal\t%%guest\t%%gnice\t%%idle\n");
    for(int i = 0; i < possible_cpus + 1; i++){
        if(!cpu_mask[i]) continue;
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

    int opt = 0;
    cpu_mask = malloc((possible_cpu + 1));
    if(!cpu_mask){
        printf("Fail to malloc cpu_mask");
        return -1;
    }
    memset(cpu_mask, 0, possible_cpu + 1);

    while(++opt < argc){
        if(!strcmp(argv[opt], "-P")){
            if (!argv[++opt]) {
				usage(argv[0]);
			}
            if(parse_values(argv[opt], cpu_mask, possible_cpu , KEY_all) < 0){
                printf("Fail to parse cpu mask");
                return -1;
            }
        }
    }

    struct cpustat* cpus_stat = malloc((possible_cpu + 1) * sizeof(struct cpustat));
    if(cpus_stat == NULL){
        printf("CPUS_STAT is NULL");
        return -1;
    }
    struct os_stat* os_stat = malloc(sizeof(struct os_stat));
    ioctl(driver, GET_OS_STAT, os_stat);
    write_header(os_stat, possible_cpu);
    ioctl(driver, GET_CPU_STAT_ALL, cpus_stat);
    unsigned long long* tot_jiffies = get_global_cpu_mpstats(cpus_stat, possible_cpu);
    struct cpu_percent_stat* cpus_percent_stat = get_percentage_stat(cpus_stat, possible_cpu, tot_jiffies);
    write_cpu_percent_stat(cpus_percent_stat, possible_cpu);
    free(tot_jiffies);
    free(cpus_percent_stat);
    free(cpus_stat);
    free(os_stat);
    close(driver);
    return 0;
}