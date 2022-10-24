// poll(), struct pollfd
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/*
typedef struct pollfd {
        int fd;                  // 需要被检测或选择的文件描述符
        short events;            // 对文件描述符fd上感兴趣的事件 
        short revents;           // 文件描述符fd上当前实际发生的事件
} pollfd_t;
*/

/*
int poll(struct pollfd fds[], nfds_t nfds, int timeout)；

args:

fds：是一个struct pollfd结构类型的数组，用于存放需要检测其状态的Socket描述符；
每当调用这个函数之后，系统不会清空这个数组，操作起来比较方便；
特别是对于 socket连接比较多的情况下，在一定程度上可以提高处理的效率；
这一点与select()函数不同，调用select()函数之后，select() 函数会清空它所检测的socket描述符集合，
导致每次调用select()之前都必须把socket描述符重新加入到待检测的集合中；
因此，select()函数适合于只检测一个socket描述符的情况，而poll()函数适合于大量socket描述符的情况；

nfds：nfds_t类型的参数，用于标记数组fds中的结构体元素的总数量；

timeout：是poll函数调用阻塞的时间，单位：毫秒；


return: 

>0：数组fds中准备好读、写或出错状态的那些socket描述符的总数量；

==0：数组fds中没有任何socket描述符准备好读、写，或出错；此时poll超时，超时时间是timeout毫秒；
换句话说，如果所检测的 socket描述符上没有任何事件发生的话，
那么poll()函数会阻塞timeout所指定的毫秒时间长度之后返回，
如果timeout==0，那么 poll() 函数立即返回而不阻塞，
如果timeout==INFTIM，那么poll() 函数会一直阻塞下去，直到所检测的socket描述符上的感兴趣的事件发生是才返回，如果感兴趣的事件永远不发生，那么poll()就会永远阻塞下去；

-1：  poll函数调用失败，同时会自动设置全局变量errno；
*/
static char *FILENAME = "/dev/polldev/dev2";

int main(int argc, char const *argv[])
{
    struct pollfd fds[4];
    int timeout_ms;
    int fd, ret;
    char readbuf[100];

    timeout_ms = atoi(argv[1]);  // -1  表示永久 阻塞 

    fd = open(FILENAME, O_RDWR);

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    ret = poll(fds, 1, timeout_ms);
    if ((ret ==1) && (fds[0].revents & POLLIN)) {
        read(fd, &readbuf, 50);
        printf("get data: %s\n", readbuf);
    } else {
        printf("not fds get ready, timeout!\n");
    }
    
    return 0;
}
