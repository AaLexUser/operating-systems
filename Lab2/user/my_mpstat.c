#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/os_lab"

int main(int argc, char *argv[]){
    int driver = open(DEVICE_PATH, O_RDWR);
    if(driver < 0){
        printf("Fail to open device.");
        return -1;
    }

    return 0;
}