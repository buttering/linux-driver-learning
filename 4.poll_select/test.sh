#!/bin/zsh

dmesg_and_clean(){
    dmesg
    sudo dmesg -C
}

# 配置文件名
if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit     
fi
test_file_name=$1
if [ $test_file_name = "async" ]
then
    module_name="async"
    # /dev目录下的chrdevice
    device_name="asyncdevice"
    chrdev_name="asyncdev"
    class_name="asyncclass"
    testfile=""
else 
    echo "\033[31mnot match file name!\033[31m"
    exit
fi
echo -n "\033[33mmodule: \033[0m"
echo $module_name
echo -n "\033[33mdevice: \033[0m"
echo $device_name

# 运行clean
sh clean.sh $test_file_name
if [ $? -ne 0 ]
then
    exit
fi
echo "\033[33m---OLD DRIVER CLEARED---\n\033[0m"

echo -n "\033[33mkernal: \033[0m"
uname -r

# 挂载驱动程序，lsmod显示/proc/modules目录下的节点
sudo insmod $module_name.ko
echo -n "\033[33mload driver: \033[0m"
lsmod | grep $module_name

# 显示已由create_device注册好的设备文件
echo -n "\033[33mload device node: \033[0m"
ls -l /dev/$device_name

group="wang"
sudo chgrp $group /dev/$device_name
sudo chmod 664 /dev/$device_name
echo -n "\033[33mdevice node change group: \033[0m"
ls -l /dev/$device_name

dmesg_and_clean

echo "\033[33m----驱动加载成功!----\033[0m"
echo -n "\033[33m/proc/device field: \033[0m"
cat /proc/devices | grep $chrdev_name
echo -n "\033[33m/sys/class node: \033[0m"
ls -l /sys/class | grep $class_name
echo -n "\033[33m/dev node: \033[0m"
ls -l /dev | grep $device_name


echo "\033[33m可用的kill信号: \033[0m"
/bin/kill -L

