# LINUX 设备驱动程序学习测试代码

## 项目说明

本项目展示作者研读《LINUX设备驱动程序》所编写的所有测试代码。
已实现驱动功能：

1. 驱动程序的挂载、卸载
2. 驱动程序的注册
3. 对驱动程序的基本系统调用
4. 在/proc下挂载驱动程序
5. 阻塞操作
6. ioctl调用
7. poll调用

## 总体运行方法

### 1. 根据文件修改Makefile注释

只需根据所运行的不同文件注释文件开头的变量。
具体修改方法见测试方法说明

### 2. 编译驱动 提供执行权限

```shell
cd HELLODRIVER
make
chmod +x test.sh
chmod +x clean.sh
```

### 3. 编译测试文件

```shell
gcc ${testfile_name}.c -o $testfile
```

testfile_name是需要运行的测试文件名，具体见测试方法说明

### 4. 运行test.sh进行测试

```shell
sh ./test.sh ${testfile_name}
```

### 5. 清理驱动

```shell
sh ./clean.sh ${testfile_name}
```

## 具体测试方法

### 1.hello_driver.c

本文件完成驱动程序基本的注册、挂载和卸载，并测试file_operations结构体的使用(读取、写入操作）。

1. 进入目录

```shell
cd 1.hello_driver
chmod +x test.sh
chmod +x clean.sh
```

2.编译驱动

```shell
make
```

3.运行测试文件

```shell
sh test.sh hello_driver
```

4.运行结果

```shell
❯ sh test.sh hello_driver
module: hello_driver
device: hello_dev
[ 2315.228068] Goodbye, World!
---OLD DRIVER CLEARED---

kernal: 5.15.0-50-generic
load driver: 506 hello_dev
load device node: crw-r--r-- 1 root root 506, 0 10月 25 14:50 /dev/hello_dev
device node change group: crw-rw-r-- 1 root wang 506, 0 10月 25 14:50 /dev/hello_dev
[ 2315.331563] Hello, world
[ 2315.331571] The process is "insmod" (pid 10133)
[ 2315.331575] Hello, Mom
[ 2315.331576] Hello, Mom
[ 2315.331578] Hello, Mom
[ 2315.331581] dev major = 506, minor = 0
向驱动读取数据
App read data:This is the kernel data
[ 2315.557830] dev file opened
[ 2315.557842] [read task]count=50
[ 2315.557845] [read data]output string: This is the kernel data
[ 2315.557968] dev file closed
向驱动写入数据
[ 2315.576305] dev file opened
[ 2315.576322] [write data]input string: This is user data!
[ 2315.576328] dev file closed
```

6.卸载和清理驱动

```shell
sh ./clean.sh hello_driver
make clean
```

### 2. proc_interface.c

测试proc接口的挂载操作，并尝试从/proc接口中读取信息。

1.进入目录并授权

```makefile
cd 2.proc_interface
chmod +x test.sh
chmod +x clean.sh
```

2.编译驱动并运行测试脚本

```shell
make
```

3.运行测试脚本

```shell
sh test.sh proc_interface
```

4.运行结果

```shell
❯ sh test.sh proc_interface
module: proc_interface
device: hello_dev
proc: hello_proc
rm: 无法删除 '/dev/hello_dev': 没有那个文件或目录
rmmod: ERROR: Module proc_interface is not currently loaded
---OLD DRIVER CLEARED---

kernal: 5.15.0-50-generic
load driver: 506 hello_dev
load device node: crw-r--r-- 1 root root 506, 0 10月 25 19:33 /dev/hello_dev
proc information is: This is a /proc interface output.
[ 1708.649623] unregister_chrdev_region
[ 1934.897420] Hello, world
[ 1934.897425] The process is "insmod" (pid 10126)
[ 1934.897428] dev major = 506, minor = 0
[ 1934.926393] proc interface is called

```

5.卸载和清理驱动

```shell
sh ./clean.sh proc_interface
make clean
```

### 3. wait_wakeup_ioctl.c

测试linux内核的阻塞与唤醒操作和原子化操作，以及测试ioctl调用

1.进入目录并授权测试脚本

```makefile
cd 3.wait_wakeup_ioctl
chmod +x test.sh
chmod +x clean.sh
```

2.编译驱动

```shell
make
```

3.运行测试脚本

```shell
sh test.sh wait_wakeup_ioctl
```

4.运行结果

```shell
❯ sh test.sh wait_wakeup_ioctl
module: wait_wakeup_ioctl
device: wwdev
testfile: test_ioctl
rm: 无法删除 '/dev/wwdev': 没有那个文件或目录
rmmod: ERROR: Module wait_wakeup_ioctl is not currently loaded
rm: 无法删除 'test_ioctl': 没有那个文件或目录
---OLD DRIVER CLEARED---

kernal: 5.15.0-50-generic
load driver: 506 wwdev
load device node: crw-r--r-- 1 root root 506, 0 10月 25 20:13 /dev/wwdev
device node change group: crw-rw-r-- 1 root wang 506, 0 10月 25 20:13 /dev/wwdev

sub process read operation to wait... 
write operation to wakeup sub process... 
[ 3734.409463] [writer]process 14072 (sh) awakening the readers...
[ 3734.410665] [reader]process 14103 (cat) going to sleep
[ 3734.410674] [reader]awoken 14103 (cat)

传入指针由ioctl修改值
ioctl read result: 55
[ 3734.505092] ioctl called!
[ 3734.505095] [ioctl read] output number to ptr: 55
直接接受ioctl调用返回值
ioctl read result: 55
[ 3734.521584] ioctl called!
[ 3734.521588] [ioctl read] output number by return: 55
```

5.继续尝试和写入读取操作

通过读取操作实现阻塞，可开启多个终端运行

```shell
cat /dev/wwdev
```

另开一个终端，通过写入操作，实现进程的唤醒，每次调用唤醒一个被阻塞的进程

```shell
echo 1 > /dev/wwdev
dmesg
```

6.卸载和清理驱动

```shell
sh clean.sh wait_wakeup_ioctl
make clean
```

### 4. poll_select.c

实验了poll系统调用的各种情况，使用class_create完成自动节点注册和多节点挂载

1.进入目录，提供权限

```shell
cd poll_select
chmod +x test.sh
chmod +x clean.sh
```

2.编译驱动

```shell
make
```

3.运行测试脚本

```shell
sh test.sh poll_select
```

4.运行结果

```shell
❯ sh test.sh poll_select
module: poll_select
device: dev
[ 3757.121941] this is dev_module_cleanup
---OLD DRIVER CLEARED---

kernal: 5.15.0-50-generic
load driver: poll_select            16384  0
load device node: 总用量 0
crw------- 1 root root 506, 0 10月 24 21:40 dev0
crw------- 1 root root 506, 1 10月 24 21:40 dev1
crw------- 1 root root 506, 2 10月 24 21:40 dev2
crw------- 1 root root 506, 3 10月 24 21:40 dev3
device node change group: 总用量 0
crw-rw-r-- 1 root wang 506, 0 10月 24 21:40 dev0
crw-rw-r-- 1 root wang 506, 1 10月 24 21:40 dev1
crw-rw-r-- 1 root wang 506, 2 10月 24 21:40 dev2
crw-rw-r-- 1 root wang 506, 3 10月 24 21:40 dev3
[ 3757.169212] Test call poll
[ 3757.169214] The process is "insmod" (pid 10607)
[ 3757.169217] dev major = 506, minor = 0, count = 4

poll调用前资源已就绪，只调用poll一次
[ 3757.254376] [write]resources are parpared (switch flag to 1)
get data: This is the kernal data!
[ 3757.261365] poll interface has been called
[ 3757.261367] resource are ready

不进行唤醒，且规定时间内(5s)资源未就绪，在调用时和超时时各调用一次
[ 3757.268024] [write]resources are scarded (switch flag to 0)
not fds get ready, timeout!
[ 3757.274508] poll interface has been called
[ 3757.274510] resource are not ready
[ 3762.277128] poll interface has been called
[ 3762.277138] resource are not ready

主动进行唤醒，且唤醒时资源已就绪 , 在调用时和被唤醒时各调用一次
[ 3762.304176] [write]resources are scarded (switch flag to 0)
get data: This is the kernal data!
[ 3762.318506] poll interface has been called
[ 3762.318509] resource are not ready
[ 3762.328979] [write]resources are parpared (switch flag to 1) and arouse initiatively
[ 3762.329023] poll interface has been called
[ 3762.329025] resource are ready

主动进行唤醒，但唤醒时资源未就绪，在调用时和被唤醒时各调用一次，会保持阻塞状态直到超时或再次被唤醒
[ 3762.336905] [write]resources are scarded (switch flag to 0)
[ 3762.345125] poll interface has been called
[ 3762.345127] resource are not ready
[ 3762.355501] [write]resources are scarded (switch flag to 0) and arouse initiatively
[ 3762.355545] poll interface has been called
[ 3762.355547] resource are not ready
not fds get ready, timeout!
```

5.清理环境

```shell
sh clean.sh poll_select
make clean
```

## 实验环境

操作系统：Linux Li-Jiancheng-Ubuntu 5.15.0-50-generic #56~20.04.1-Ubuntu SMP Tue Sep 27 15:51:29 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux

IDE：VSCode
