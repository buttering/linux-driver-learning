// 下面三个：read(), write(), close()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// perror(),printf()
#include <stdio.h>
// sigaction(), struct sigaction
#include <signal.h>
// getpid()
#include <unistd.h>

#define MAX_BUF_LEN 100
static const char* DEVICE_PATH = "/dev/asyncdevice";

void sig_handler(int sig){
    printf("[app]receive signal: %d\n", sig);
}

int main(int argc, char const *argv[]){
    struct sigaction act;
    int readFd;
    int ret;
    printf("[app]Pid: %d\n", getpid());
    readFd = open(DEVICE_PATH, O_RDWR, 666);
    if (readFd < 0){
        // 打印上一个函数的错误
        perror("[app]Open failed\n");
        return -1;
    }
    printf("[app]Driver file open successed: %d\n", readFd);


    // signal(SIGIO, &sig_handler);  推荐使用sigaction函数


    act.sa_handler = sig_handler;  // 指定处理函数(信号捕捉函数)
    // 清空sa_mask(信号捕捉函数执行期间屏蔽的信号集)并将信号SiGQUIT添加，结果使程序屏蔽SIGQUIT中断
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGQUIT);
    act.sa_flags = 0;  // 通常设置为0,表示使用默认属性(屏蔽重复信号,即使没有设置sa_mask)
    ret = sigaction(SIGIO, &act, NULL);  // 使程序捕捉SIGIO(29)信号
    printf("[app]Listion signal: SIGIO(%d)\n", SIGIO);
    if (ret < 0){
        perror("[app]Sigaction error\n");
        return -1;
    }
    ret = sigaction(SIGUSR1, &act, NULL);  // 使程序捕捉SIGUSR1(10)信号
    printf("[app]Listion signal: SIGUSR1(%d)\n", SIGUSR1);
    if (ret < 0){
        perror("[app]Sigaction error\n");
        return -1;
    }

    fcntl(readFd, F_SETOWN, getpid());  // 指定当前进程为打开文件的属主
    // fcntl(STDIN_FILENO, F_SETOWN, getpid());  // 也可以指定为标准输入设备的属主3
    int oflags = fcntl(readFd, F_GETFL);  // 获取文件的flags，即open的第二个参数
    fcntl(readFd, F_SETFL, oflags | FASYNC);  // 在设备标识符中设置FASYNC表示，启用异步通知机制

    // 进入忙等
    while(1){
        sleep(2);
    }
}