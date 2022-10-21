#include <linux/init.h>
// THIS_MODULE
#include <linux/module.h>
#include <linux/kernel.h>
// current
#include <linux/sched.h>
#include <linux/moduleparam.h>
// struct inode, struct file, cdev_del()
#include <linux/cdev.h>
// dev_t
#include <linux/types.h>
// struct file_operaction, register_chrdev_region(), alloc_chrdev_region(), unregister_chrdev_region()
#include <linux/fs.h>
// MAJOR, MINOR, MKDEV
#include <linux/kdev_t.h>
// copy_to_user()
#include <linux/uaccess.h>
// kfree(), kmalloc()
#include <linux/slab.h>
// printk_ratelimited
#include <linux/ratelimit.h>

// 声明模块参数
static char *whom = "world";
static int howmany = 1;
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);


// 设备结构体
struct hello_dev{
    struct cdev cdev;   /*字符设备*/
    dev_t devid;        /*设备号*/
    int major;          /*主设备号*/
    int minor;          /*次设备号*/
};

struct hello_dev dev;

//  字符设备操作数据结构
static char readbuf[100];
static char writebuf[100];
static char kerneldata[] = {"This is the kernel data"};

// 定义字符设备操作函数
static int hello_open(struct inode *inode, struct file *filp){
    return 0;
}

static int hello_release(struct inode *inode, struct file *filp){
    return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos){
    int ret = 0;
    // memcpy(readbuf, kerneldata, sizeof(kerneldata));
    printk_ratelimited(KERN_INFO "[read task]count=%ld\n", count);
    memcpy(readbuf, kerneldata, sizeof(kerneldata));

    ret = copy_to_user(buf, readbuf, count);
    printk_ratelimited(KERN_INFO "[read data]output string: %s\n", readbuf);  // 限制打印速率，但不知为何没有效果

    return count;
}

static ssize_t hello_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos){
    int ret = 0;
    // char* buf_kernal = kmalloc(sizeof(char) * count, GFP_KERNEL);
    // ret = copy_from_user(buf_kernal, buf, count);
    // strcpy(writebuf, buf_kernal);
    ret = copy_from_user(writebuf, buf, count);
    if (ret == 0){
        printk(KERN_INFO "[write data]input string: %s\n", writebuf);
    }
    return count-1;  // 应返回成功写入的字节数，如果返回值小于count，内核会不断重复wtire调用
}

// 初始化文件操作结构体
static const struct file_operations hello_fops = {
        .owner = THIS_MODULE,
        .open = hello_open,
        .release = hello_release,
        .read = hello_read,
        .write = hello_write
};

static void setup_cdev(struct hello_dev *dev) {

    cdev_init(&dev->cdev, &hello_fops);
    int err = cdev_add(&dev->cdev, dev->devid, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding\n", err);
}

// __init标记表示函数经在初始化使用，初始化后会丢弃函数，释放内存
static int __init hello_init(void) {
    printk(KERN_ALERT "Hello, world\n");
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
    int i = 0;
    while (i < howmany) {
        printk(KERN_INFO "Hello, %s", whom);
        i ++;
    }

    // 分配设备编号
    int ret = 0;
    if (dev.major) {  // 静态注册设备号
        dev.devid = MKDEV(dev.major, 0);
        ret = register_chrdev_region(dev.devid, 1, "hello_dev");
    } else {  // 动态注册设备号
        ret = alloc_chrdev_region(&dev.devid, 0, 1, "hello_dev");
        dev.major = MAJOR(dev.devid);
        dev.minor = MINOR(dev.devid);
    }

    if (ret < 0) {
        printk(KERN_ALERT "dev chrdev_region err!\r\n");    
        return -1;
    }
    printk("dev major = %d, minor = %d\r\n",dev.major, dev.minor);

    // 注册字符设备
    setup_cdev(&dev);

    return 0;
}

// __exit表示函数仅用于模块卸载
static void __exit hello_exit(void){
    printk(KERN_ALERT "Goodbye, World!\n");

    // 删除字符设备
    cdev_del(&dev.cdev);
    // 注销字符设备
    unregister_chrdev_region(dev.devid, 1);
}

// 特殊宏，指明初始化函数和清除函数的位置
module_init(hello_init);
module_exit(hello_exit);

// 模块描述性定义
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");