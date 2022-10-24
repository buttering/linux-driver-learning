#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
// DECLARE_WAIT_QUEUE_HEAD(), wait_queue_head_t
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/version.h>
// struct semaphore
#include <linux/semaphore.h>
// contain_of()
#include <linux/kernel.h>


#define WW_IOC_MAGIC 'w'  // ioctl幻数
#define WW_IOCTEST _IOR(WW_IOC_MAGIC, 0, char*)
#define WW_IOCTEST_PTR _IOR(WW_IOC_MAGIC, 1, int)
#define WW_IOC_MAXNR 1

//  ioctl测试
static int kernelnumber = 55;

struct ww_dev {
    struct cdev cdev;
    dev_t devid;
    int major;
    int minor;
    int flag;  // 休眠标志
    wait_queue_head_t wait_queue;  // 简单休眠队列
    struct semaphore sem;  // 互斥信号量
};

// 提供给驱动程序以初始化人力，从而为以后的操作完成初始化做准备
static int ww_open(struct inode *inode, struct file *filp){
    // 将设备结构体保存到file结构中，以便以后对该指针的访问
    struct ww_dev *dev;
    dev = container_of(inode->i_cdev, struct ww_dev, cdev);
    filp->private_data = dev;
    return 0;
}

static int ww_release(struct inode *inode, struct file *filp){
    return 0;
}

// read方法，用户调用此方法使进程陷入休眠
static ssize_t ww_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos){
    struct ww_dev *dev = filp->private_data;
    printk(KERN_DEBUG "[reader]process %i (%s) going to sleep\n", current->pid, current->comm);

    // 获取锁，使对flag的访问原子化
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    wait_event_interruptible(dev->wait_queue, dev->flag != 0);
    dev->flag = 0;  // 不能直接访问全局变量dev，会出现段错误，必须使用open方法初始化的
    // 释放锁
    up(&dev->sem);

    printk(KERN_DEBUG "[reader]awoken %i (%s)\n", current->pid, current->comm);

    return 0;
}


// write方法，用户调用此方法唤醒所有因read陷入休眠的进程，但只有一个进程跳出休眠
static ssize_t ww_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppost){
    struct ww_dev *dev = filp->private_data;
    printk(KERN_DEBUG "[writer]process %i (%s) awakening the readers...\n", current->pid, current->comm);

    dev->flag = 1;
    wake_up_interruptible(&dev->wait_queue);

    return count;
}

// ioctl方法，允许自定调用
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int io_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg){
#else
static long io_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    struct inode *inode = file_inode(filp);
#endif
    int retval = 0;
    printk(KERN_NOTICE "ioctl called!\n");

    /* 检测命令的有效性 */
    if (_IOC_TYPE(cmd) != WW_IOC_MAGIC){
        printk(KERN_ALERT "wrong ioc magic: \"%c\" is not equal \"%c\"!\n", _IOC_TYPE(cmd), WW_IOC_MAGIC);
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > WW_IOC_MAXNR){
        printk(KERN_ALERT "exceed the max nr\n");
        return -ENOTTY;
    }
    switch(cmd) {
        int ret;
        case WW_IOCTEST_PTR:
            ret = put_user(kernelnumber, (int __user *) arg);
            printk("[ioctl read] output number: %d\n", kernelnumber);
            break;
        case WW_IOCTEST:
            return kernelnumber;
    }

    return retval;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ww_open,
    .release = ww_release,
    .read = ww_read,
    .write = ww_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctk = io_ioctl
#else
    .unlocked_ioctl = io_ioctl
#endif
};

static void setup_cdev(struct ww_dev *dev){
    int err; 
    cdev_init(&dev->cdev, &fops);
    err = cdev_add(&dev->cdev, dev->devid, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding\n", err);
}

static int __init ww_init(void) {
    int ret;
    printk(KERN_INFO "Test wait and wake up\n");
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);

    // 初始化设备结构体
    ret = 0;
    ret = alloc_chrdev_region(&dev.devid, 0, 1, "wwdev");
    dev.major = MAJOR(dev.devid);
    dev.minor = MAJOR(dev.devid);
    dev.flag = 0;
    init_waitqueue_head(&dev.wait_queue);
    sema_init(&dev.sem, 1);
    

    if (ret < 0) {
        printk(KERN_ALERT "dev chrdev_region err!\r\n");    
        return -1;
    }
    printk("dev major = %d, minor = %d\r\n",dev.major, dev.minor);

    setup_cdev(&dev);

    return 0;
}
static void __exit ww_exit(void){
    cdev_del(&dev.cdev);
    unregister_chrdev_region(dev.devid, 1);
}

module_init(ww_init);
module_exit(ww_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");