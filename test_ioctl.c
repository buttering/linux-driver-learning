#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/ioctl.h>
// errno
#include <errno.h>
// strerror()
#include <string.h>

// 0x77="w"
#define IOCTEST _IOR(0x77, 0x0, char*)
static char *FILENAME = "/dev/wwdev";
int main(int argc, char const *argv[])
{
    int fd, ret;
    unsigned char buf[100];

    fd = open(FILENAME, O_RDWR);
    if (fd < 0)
        return -1;
    
    ret = ioctl(fd, IOCTEST, buf);
    if (ret < 0){
        printf("error: %d->%s\n", errno, strerror(errno));
    }
}
