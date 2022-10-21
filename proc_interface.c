#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
// seq_open(), seq_read(), seq_file, single_release()
#include <linux/seq_file.h>
// KERNEL_VERSION, LINUX_VERSION_CODE
#include <linux/version.h>


// 设备结构体
struct hello_dev{
    struct cdev cdev;   /*字符设备*/
    dev_t devid;        /*设备号*/
    int major;          /*主设备号*/
    int minor;          /*次设备号*/
} dev;

static int device_open(struct inode *inode, struct file *filp){
    return 0;
}
static int device_release(struct inode *inode, struct file *filp){
    return 0;
}
static ssize_t device_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos){
    return count;
}
static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos){
    return count;
}
static const struct file_operations hello_fops = {
        .owner = THIS_MODULE,
        .open = device_open,
        .release = device_release,
        .read = device_read,
        .write = device_write
};

static void setup_cdev(struct hello_dev *dev) {

    cdev_init(&dev->cdev, &hello_fops);
    int err = cdev_add(&dev->cdev, dev->devid, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding", err);
}

// 使用/proc文件系统（3.10以上）
// 1.实现show()方法，供内核将数据输出到用户空间
static int hello_proc_show(struct seq_file *s, void *v){
    seq_printf(s, "This is a /proc interface output.\n");
    return 0;
}

// 2.将show（）方法注册到file_operations结构体上
static int hello_proc_open(struct inode *inode, struct file *file){
    return single_open(file, hello_proc_show, NULL);
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_fops = {
    .proc_open = hello_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};
#else
static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = hello_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release
};
#endif


static int __init device_init(void){
    printk(KERN_ALERT "Hello, world\n");
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
       
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


// 3.使用fops结构体建立实际/proc文件
    proc_create_data(
        "hello_proc",  /*在/proc下创建的文件名称*/
        0, /*文件保护掩码， 0表示系统默认值*/
        NULL, /*文件所在目录指针*/
        &proc_fops,
        NULL
    );
    return 0;
}

static void __exit device_exit(void){
    printk(KERN_ALERT "Goodbye, World!\n");
    // 3.删除/proc入口项
    remove_proc_entry("hello_proc", NULL);
    printk("remove /proc entry");

    // 移除字符设备
    cdev_del(&dev.cdev);
    printk("remove cdev");

    // 注销字符设备, 释放设备编号
    unregister_chrdev_region(dev.devid, 1);
    printk("unregister_chrdev_region");


}


module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");