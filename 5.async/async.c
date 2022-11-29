// THIS_MODULE
#include <linux/module.h>
// current
#include <linux/sched.h>
// struct cdev, struct inode, struct file, cdev_del()
#include <linux/cdev.h>
// struct file_operaction, register_chrdev_region(), alloc_chrdev_region(), unregister_chrdev_region(), struct fasync_struct, kill_fasync()
#include <linux/fs.h>
// kfree(), kmalloc() 
#include <linux/slab.h>
// contain_of()
#include <linux/kernel.h>
// SIGIO
#include "asm/signal.h"
// IS_ERR
#include "linux/err.h"
// PULL_IN, PULL_OUT
#include "asm-generic/siginfo.h"
// copy_to_user()
#include <linux/uaccess.h>

#define MAX_BUF_LEN 100
static const char* CHRDEVNAME = "asyncdev";
static const char* CLASSNAME = "asyncclass";
static const char* DEVICENAME = "asyncdevice";

struct async_dev {

    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;  // cdev
    struct class *cdevclass;  // 字符设备逻辑类
    struct device *device;  // 设备
    char databuf[MAX_BUF_LEN];
    struct fasync_struct *fasyncQueue;  // 异步通知等待队列
}dev;

static int async_open(struct inode *inode, struct file *filp) {
    struct async_dev *dev;
    dev = container_of(inode->i_cdev, struct async_dev, cdev);
    filp->private_data = dev;
    return 0;
}

static int async_release(struct inode *inode, struct file *filp){
    return 0;
}

// 写入数据触发异步通知
static ssize_t async_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    int ret;
    int signal;
    struct async_dev *dev = filp->private_data;
    if ( count > MAX_BUF_LEN )
        count = MAX_BUF_LEN;
    
    memset(dev->databuf, 0, MAX_BUF_LEN);
    ret = copy_from_user(dev->databuf, buf, count);
    if (ret)
        return -EFAULT;
    printk("[write]Get data: %s", dev->databuf);

    /* 
    TODO:因为未知原因，驱动只能向用户程序发送SIGIO(29)信号，
    即使主动发送其他信号，用户程序接收到的依然是29号信号
     */
    // ret = kstrtouint(dev->databuf, 10, &signal);  // 转化为数字，成功返回0
    // if (ret){
    //     printk("[wirte]Data is not a number\n");
    //     signal = 0;
    // }
    // switch (signal) {
    //     case SIGIO:
    //     case SIGQUIT:
    //     case SIGUSR1:
    //     case SIGUSR2:
    //         break;
    //     default:
    //         signal = 0;  // 只能发送指定的信号
    // }

    signal = SIGIO;

    /*     
    void kill_fasync(struct fasync_struct **fp, int sig, int band)
    fp：要操作的 fasync_struct。
    sig： 要发送的信号。
    band： 可读时设置为 POLL_IN，可写时设置为 POLL_OUT。 
    */
    if (signal != 0 && dev->fasyncQueue) {
        // kill_fasync(&dev->fasyncQueue, signal, POLL_IN);
        kill_fasync(&dev->fasyncQueue, signal, POLL_IN);
        // printk("[write]Send async signal: %d\n", signal);
        printk("[write]Send async signal: %d\n", SIGIO);
    }

    return count;
}

static ssize_t async_read(struct file *filp, char __user *buf, size_t count, loff_t *pos) {
    int ret;
    struct async_dev *dev = filp->private_data;
    if (count > MAX_BUF_LEN) 
        count = MAX_BUF_LEN;
    ret = copy_to_user(buf, dev->databuf, count);
    if (ret)
        return -EFAULT;
    printk("[read]Output data: %s\n", dev->databuf);
    
    return count;
}

// 异步通知必须实现这个函数接口，将参数和等待队列传入内核方法
static int async_fasync(int fd, struct file *filp, int mode){
    struct async_dev *dev = filp->private_data;
    // fasync_struct由该函数自动完成初始化
    printk("[fasync]regist async\n");
    return fasync_helper(fd, filp, mode, &dev->fasyncQueue);
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = async_open,
    .release = async_release,
    .fasync = async_fasync,
    .write = async_write,
    .read = async_read
};

void setup_cdev(void){
    struct device *devices;
    // 注册字符设备驱动
    // 设备将被添加到/proc/device文件中
    int ret = alloc_chrdev_region(&dev.devid, 0, 1, CHRDEVNAME);
    if (ret)
        goto out;
    dev.major = MAJOR(dev.devid);
    dev.minor = MINOR(dev.devid);
    printk("[init]dev major = %d, minor = %d\n",dev.major, dev.minor);

    // 将fops注册到dev的cdev上
    cdev_init(&dev.cdev, &fops);
    // 添加字符设备，使内核能够调用该设备
    ret = cdev_add(&dev.cdev, dev.devid, 1);
    if (ret)
        goto out1;

    // class结构体在linux中表示一种设备的集合，会出现在/sys/class中
    dev.cdevclass = class_create(THIS_MODULE, CLASSNAME);
    if (IS_ERR(dev.cdevclass))
        goto out2;
    
    // 使用class创建设备，依赖于用户空间的udev工具
    // 加载模块的时候，用户空间中的udev会自动响应device_create(…)函数，去/sysfs下寻找对应的类从而创建设备节点
    // 最终在/dev下创建该设备文件
    devices = device_create(dev.cdevclass, NULL, dev.devid, NULL, DEVICENAME);
    if (devices == NULL) {
        goto out3;
    }

    return;

out3:
    class_destroy(dev.cdevclass);
out2:
    cdev_del(&dev.cdev);
out1:
    unregister_chrdev_region(dev.devid, 1);
out: 
    return;
}

static int __init async_init(void){
    printk(KERN_INFO "[init]Test async message\n");
    printk(KERN_INFO "[init]The process is \"%s\" (pid %i)\n", current->comm, current->pid);
    
    setup_cdev();

    return 0;
}

static void __exit async_exit(void){
    device_destroy(dev.cdevclass, dev.devid);  // 卸载设备
    class_destroy(dev.cdevclass);  // 删除类
    cdev_del(&dev.cdev);  // 删除cdev
    unregister_chrdev_region(dev.devid, 1);
    printk("[exit]module clean up!\n");
}


module_init(async_init);
module_exit(async_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");