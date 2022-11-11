#!/bin/zsh

# 配置文件名
if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit     
fi
test_file_name=$1
if [ $test_file_name = "proc_interface" ]
then
    module="proc_interface"
    device="hello_dev"
    proc="hello_proc"
else 
    echo "\033[31mnot match file name!\033[31m"
    exit
fi
echo -n "\033[33mmodule: \033[0m"
echo $module
echo -n "\033[33mdevice: \033[0m"
echo $device
echo -n "\033[33mproc: \033[0m"
echo $proc

# 运行clean
sh clean.sh $test_file_name
if [ $? -ne 0 ]
then
    exit
fi
echo "\033[33m---OLD DRIVER CLEARED---\n\033[0m"

echo -n "\033[33mkernal: \033[0m"
uname -r

# 挂载驱动程序
sudo dmesg -C
sudo insmod $module.ko 
echo -n "\033[33mload driver: \033[0m"
cat /proc/devices | grep $device

# 在/dev目录下注册设备文件 
major=$(awk '$2=="'$device'" {print $1}' /proc/devices)
sudo mknod /dev/${device} c $major 0
echo -n "\033[33mload device node: \033[0m"
ls -l /dev/${device} | grep $device

# 建立/proc接口
if test -n "$proc" && test -n "$(ls /proc | grep $proc)"
then 
    echo -n "\033[33mproc information is: \033[0m"
    cat /proc/${proc}
else
    echo "\033[31mproc register failure\033[0m"
fi
dmesg