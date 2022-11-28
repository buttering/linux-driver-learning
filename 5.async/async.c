// THIS_MODULE
#include "linux/err.h"
#include <linux/module.h>
// current
#include <linux/sched.h>
// struct cdev, struct inode, struct file, cdev_del()
#include <linux/cdev.h>
// struct file_operaction, register_chrdev_region(), alloc_chrdev_region(), unregister_chrdev_region()
#include <linux/fs.h>
// kfree(), kmalloc() 
#include <linux/slab.h>
// contain_of()
#include <linux/kernel.h>

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
    char readbuf[MAX_BUF_LEN];
    char writebuf[MAX_BUF_LEN];
}dev;

static int async_open(struct inode *inode, struct file *filp) {
    struct async_dev *pdev;
    pdev = container_of(inode->i_cdev, struct async_dev, cdev);
    filp->private_data = pdev;
    return 0;
}

static int async_release(struct inode *inode, struct file *filp){
    return 0;
}

struct file_operations fops = {
    .open = async_open,
    .release = async_release
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
    printk("dev major = %d, minor = %d\n",dev.major, dev.minor);

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
    printk(KERN_INFO "Test async message\n");
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
    
    setup_cdev();

    return 0;
}

static void __exit async_exit(void){
    device_destroy(dev.cdevclass, dev.devid);  // 卸载设备
    class_destroy(dev.cdevclass);  // 删除类
    cdev_del(&dev.cdev);  // 删除cdev
    unregister_chrdev_region(dev.devid, 1);
    printk("module clean up!\n");
}


module_init(async_init);
module_exit(async_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");