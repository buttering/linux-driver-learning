#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/ioctl.h>
// errno
#include <errno.h>
// strerror()
#include <string.h>

// 0x77="w"
#define IOCTEST_VALUE _IOR(0x77, 0x0, char*)
#define IOCTEST_PTR _IOR(0x77, 0x1, int)

static char *FILENAME = "/dev/wwdev";

int main(int argc, char const *argv[])
{
    int fd, ret;
    unsigned char buf[100];
    int ptrnum, num;

    fd = open(FILENAME, O_RDWR);
    if (fd < 0)
        return -1;
    
    ret = ioctl(fd, IOCTEST_PTR, &ptrnum);
    num = ioctl(fd, IOCTEST_VALUE);
    if (ret < 0){
        printf("error: %d->%s\n", errno, strerror(errno));
        return 0;
    }
    printf("ioctl read result: %d\n", ptrnum);
    printf("ioctl read result: %d\n", num);
}
