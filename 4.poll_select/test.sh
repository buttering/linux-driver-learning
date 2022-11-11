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
if [ $test_file_name = "poll_select" ]
then
    module="poll_select"
    devicefolder="polldev"
    device="dev"
    testfile="test_poll"
else 
    echo "\033[31mnot match file name!\033[31m"
    exit
fi
echo -n "\033[33mmodule: \033[0m"
echo $module
echo -n "\033[33mdevice: \033[0m"
echo $device


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
sudo insmod $module.ko 
echo -n "\033[33mload driver: \033[0m"
lsmod | grep $module

# 在/dev目录下注册  使用create_class 和 create_device 无需使用mknod手动注册
echo -n "\033[33mload device node: \033[0m"
ls -l /dev/${devicefolder}

# 提供权限，这里为四个子设备添加组内读写权限
group="wang"
sudo chgrp $group /dev/${devicefolder}/${device}[0-3]
sudo chmod 664 /dev/${devicefolder}/${device}[0-3]
echo -n "\033[33mdevice node change group: \033[0m"
ls -l /dev/${devicefolder}

dmesg_and_clean

# 编译测试文件
gcc ${testfile}.c -o ${testfile}

echo "\033[33m\npoll调用前资源已就绪，只调用poll一次\033[0m"
echo "yt" > /dev/${devicefolder}/${device}2
dmesg_and_clean
./${testfile} -1
dmesg_and_clean

echo "\033[33m\n不进行唤醒，且规定时间内(5s)资源未就绪，在调用时和超时时各调用一次\033[0m"
echo "nt" > /dev/${devicefolder}/${device}2
dmesg_and_clean
./${testfile} 5000
dmesg_and_clean

echo "\033[33m\n主动进行唤醒，且唤醒时资源已就绪 , 在调用时和被唤醒时各调用一次\033[0m"
echo "nt" > /dev/${devicefolder}/${device}2
dmesg_and_clean
./${testfile} -1 &
sleep 0.01
echo "yb" > /dev/${devicefolder}/${device}2
dmesg_and_clean

echo "\033[33m\n主动进行唤醒，但唤醒时资源未就绪，在调用时和被唤醒时各调用一次，会保持阻塞状态直到超时或再次被唤醒\033[0m"
echo "nt" > /dev/${devicefolder}/${device}2
dmesg_and_clean
./${testfile} 5000 &
sleep 0.01
echo "nb" > /dev/${devicefolder}/${device}2
dmesg_and_clean
wait