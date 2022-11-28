#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/semaphore.h>
// device_create(), class_create()
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>

static const int CDEVCOUNT = 4;  // 子设备数
static const char *CDEVNAME = "polldev";
static const char *INODENAME = "pollinode";

static char readbuf[100], writebuf[100];
static char data[] = "This is the kernal data!";

struct class *cdevclass = NULL;  // 字符设备逻辑类

struct poll_dev{
    struct cdev cdev;
    dev_t devid;
    int major;
    int minor;
    wait_queue_head_t wait_queue;
    struct semaphore sem;
    int flagin;  // 模拟读资源
}dev;

static int poll_open(struct inode *inode, struct file *filp){
    struct poll_dev *pdev;
    pdev = container_of(inode->i_cdev, struct poll_dev, cdev);
    filp->private_data = pdev;
    return 0;
}

static int poll_release(struct inode *inode, struct file *filp){
    return 0;
}

static ssize_t poll_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos){
    int ret;
    memcpy(readbuf, data, sizeof(data));
    ret = copy_to_user(buf, readbuf, count);
    return count;
}

static ssize_t poll_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
    struct poll_dev *dev = filp->private_data;
    int ret;

    ret = copy_from_user(writebuf, buf, count);
    /* 
    wirtebuf[0]:
        "n": not get ready for read
        "y": get ready for read
    writebuf[1]:
        "t": wake up until timeout
        "b": wake up initiatively
    */
    if (writebuf[0] == 'n' && writebuf[1] == 't') {
        printk("[write]resources are scarded (switch flag to 0)\n");
        dev->flagin = 0;
    }else if (writebuf[0] == 'y' && writebuf[1] == 't') {
        printk("[write]resources are parpared (switch flag to 1)\n");
        dev->flagin = 1;
    }else if (writebuf[0] == 'n' && writebuf[1] == 'b') {
        printk("[write]resources are scarded (switch flag to 0) and arouse initiatively\n");
        dev->flagin = 0;
        wake_up_interruptible(&dev->wait_queue);
    }else if (writebuf[0] == 'y' && writebuf[1] == 'b') {
        printk("[write]resources are parpared (switch flag to 1) and arouse initiatively\n");
        dev->flagin = 1;
        wake_up_interruptible(&dev->wait_queue);
    }
    return count;
}

// poll调用接口，查看是否可以读取数据
static unsigned int poll_poll(struct file* filp, struct poll_table_struct *wait){
    struct poll_dev *dev = filp->private_data;
    int mask = 0;  //  位掩码

    printk("[poll]poll interface has been called\n");
    down(&dev->sem);
    poll_wait(filp, &dev->wait_queue, wait);
    
    if (dev->flagin){
        printk("[poll]resource are ready\n");
        mask |= POLLIN | POLLRDNORM;  // 可读取
    } else {
        printk("[poll]resource are not ready\n");
    }
    up(&dev->sem);
    return mask;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = poll_open,
    .release = poll_release,
    .read = poll_read,
    .write = poll_write,
    .poll = poll_poll
};

static void setup_cdev(void){
    int count;
    // 动态申请设备号 
    int ret = alloc_chrdev_region(&dev.devid, 0, CDEVCOUNT, CDEVNAME);
    if (ret)
        return;
    dev.major = MAJOR(dev.devid);
    dev.minor = MINOR(dev.devid);
    printk("dev major = %d, minor = %d, count = %d\r\n",dev.major, dev.minor, CDEVCOUNT);
    // 初始化cdev结构体
    // dev.cdev = cdev_alloc();
    // if (!dev.cdev)
    //     goto out;
    cdev_init(&dev.cdev, &fops);

    // 添加字符设备到系统中，第二个参数是第一个设备的编号 
    ret = cdev_add(&dev.cdev, dev.devid, CDEVCOUNT);
    if (ret)
        goto out1;

    //  创建设备类，无需再调用mknod指令手动创建/dev节点
    cdevclass = class_create(THIS_MODULE, INODENAME);
    if (IS_ERR(cdevclass))
        goto out2;
    for (count = 0; count < CDEVCOUNT; count ++)
        device_create(cdevclass, NULL, dev.devid + count, NULL, "polldev/dev%d", count);
    
// out:
//     unregister_chrdev_region(dev.devid, CDEVCOUNT);
//     return;
out1:
    unregister_chrdev_region(dev.devid, CDEVCOUNT);
    // kfree(&dev.cdev);
    return;
out2:
    cdev_del(&dev.cdev);
    unregister_chrdev_region(dev.devid, CDEVCOUNT);
    // kfree(&dev.cdev);
    return;
}

static int __init poll_init(void) {
    // int ret = 0;
    printk(KERN_INFO "Test call poll\n");
    printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);

    // 初始化设备结构体，共创建四个设备

    init_waitqueue_head(&dev.wait_queue);    
    sema_init(&dev.sem, 1);
    dev.flagin = 0;

    setup_cdev();

    return 0;
}
static void __exit poll_exit(void){
    int count;
    for (count = 0; count < CDEVCOUNT; count ++) 
        device_destroy(cdevclass, dev.devid + count);
    class_destroy(cdevclass);
    cdev_del(&dev.cdev);
    unregister_chrdev_region(dev.devid, CDEVCOUNT);
    // kfree(&dev.cdev);
    printk("this is dev_module_cleanup\n");
}

module_init(poll_init);
module_exit(poll_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wang Jiawei");