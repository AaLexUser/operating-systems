#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define IOC_MAGIC 'a'

#define WR_VALUE _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define RD_VALUE _IOR(IOC_MAGIC, 1, struct ioctl_arg)


#define DEVICE_PATH "/dev/os_lab"

struct ioctl_arg{
    unsigned int val;
};

int main(int argc, char *argv[]){
    int driver = open(DEVICE_PATH, O_RDWR);
    if(driver < 0){
        printf("Fail to open device.\n");
        return -1;
    }
    struct ioctl_arg umsg;
    ioctl(driver, RD_VALUE, &umsg);
    close(driver);
    return 0;
}