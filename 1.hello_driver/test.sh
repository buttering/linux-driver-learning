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
if [ $test_file_name = "hello_driver" ]
then
    module="hello_driver"
    device="hello_dev"
    testfile="test_driver"
else 
    echo "\033[31mnot match file name!\033[31m"
    exit
fi
echo -n "\033[33mmodule: \033[0m"
echo $module
echo -n "\033[33mdevice: \033[0m"
echo $device

# 运行clean
./clean.sh $test_file_name
if [ $? -ne 0 ]
then
    exit
fi
echo "\033[33m---OLD DRIVER CLEARED---\n\033[0m"

echo -n "\033[33mkernal: \033[0m"
uname -r

# 挂载驱动程序
sudo dmesg -C
sudo insmod $module.ko howmany=3 whom="Mom"
echo -n "\033[33mload driver: \033[0m"
cat /proc/devices | grep $device

# 在/dev目录下注册设备文件 
major=$(awk '$2=="'$device'" {print $1}' /proc/devices)
sudo mknod /dev/${device} c $major 0
echo -n "\033[33mload device node: \033[0m"
ls -l /dev/${device} | grep $device

# 提供权限
group="wang"
sudo chgrp $group /dev/${device}
sudo chmod 664 /dev/${device}
echo -n "\033[33mdevice node change group: \033[0m"
ls -l /dev/${device} | grep $device

dmesg_and_clean

# 读写测试脚本
gcc ${testfile}.c -o ${testfile}

echo "\033[33m向驱动读取数据\033[0m"
./${testfile} 1
dmesg_and_clean

echo "\033[33m向驱动写入数据\033[0m"
./${testfile} 2
dmesg_and_clean
