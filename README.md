# LINUX 设备驱动程序学习测试代码

## 项目说明

本项目展示作者研读《LINUX设备驱动程序》所编写的所有测试代码。
已展示驱动功能：

1. 驱动程序的挂载、卸载
2. 驱动程序的注册
3. 在/proc下挂载驱动程序
4. 阻塞操作
5. ioctl操作

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

本文件完成驱动程序基本的注册、挂载和卸载，并测试file_operations结构体的使用。

1. Makefile取消注释：

```makefile
obj-m += hello_driver.o
```

2. 编译驱动并运行测试脚本

```shell
make
sh ./test.sh hello_driver
```

3. 编译测试文件

```shell
gcc test_driver.c -o test_driver
```

4. 读取操作

```shell
./test_driver 1
```

5. 写入操作

```shell
./test_driver 2
dmesg
```

6. 卸载驱动

```shell
sh ./clean.sh hello_driver
make clean
```

### 2. proc_interface.c

测试proc接口的挂载操作，并尝试从/proc接口中读取信息。

1. Makefile取消注释：

```makefile
obj-m += proc_interface.o
```

2. 编译驱动并运行测试脚本

```shell
make
sh ./test.sh proc_interface
```

3. 卸载驱动

```shell
sh ./clean.sh proc_interface
make clean
```

### wait_wakeup_ioctl.c

测试linux内核的阻塞与唤醒操作和原子化操作，以及测试ioctl调用

1. Makefile取消注释：

```makefile
obj-m += wait_wakeup_ioctl.o
```

2. 编译驱动并运行测试脚本

```shell
make
sh ./test.sh wait_wakeup_ioctl
```

3. 编译测试文件

```shell
gcc test_ioctl.c -o test_ioctl
```

4. 读取操作，实现阻塞，可开启多个shell测试

```shell
cat /dev/wwdev
```

5. 写入操作，实现进程的唤醒，每次调用唤醒一个被阻塞的进程

```shell
echo 1 > /dev/wwdev
dmesg
```

6. ioctl 调用

```shell
./test_ioctl
```

7. 卸载驱动

```shell
sh ./clean.sh wait_wakeup_ioctl
make clean
```

## 实验环境

操作系统：Linux Li-Jiancheng-Ubuntu 5.15.0-50-generic #56~20.04.1-Ubuntu SMP Tue Sep 27 15:51:29 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux
IDE：VSCode
