#include <sys/types.h>//这里提供类型pid_t和size_t的定义
#include <sys/stat.h>
// open()
#include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(int argc, char const *argv[])
{
    int ret = 0;
    int fd = 0;
    
    char *filename = "/dev/hello_dev";
    char readbuf[100], writebuf[100];
    static char userdata[] = {"This is user data!"};

    fd = open(filename, O_RDWR);
    if (fd < 0){
        goto out;
    }

    if (atoi(argv[1]) == 1){    
        ret = read(fd, readbuf, 50);
        if (ret < 0){
            printf("read file %s failed!\r\n", filename);
        } else {
            printf("App read data:%s\r\n", readbuf);
        }
    }

    if (atoi(argv[1]) == 2){
        memcpy(writebuf, userdata, sizeof(userdata));
        ret = write(fd, writebuf, 50);
        if (ret < 0){
            printf("write file %s failed\r\n", filename);
        }
    }


out:
    close(fd);
    return 0;
}
