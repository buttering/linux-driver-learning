#!/bin/zsh


if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit 1
fi
test_file_name=$1
if [ $test_file_name = "hello_driver" ]
then
    module="hello_driver"
    device="hello_dev"
elif [ $test_file_name = "proc_interface" ]
then
    module="proc_interface"
    device="hello_dev"
elif [ $test_file_name = "wait_wakeup_ioctl" ]
then
    module="wait_wakeup_ioctl"
    device="wwdev"
elif [ $test_file_name = "poll_select" ]
then
    module="poll_select"
    device="polldev"
else    
    exit 1
fi

sudo dmesg -C
sudo rm -r /dev/${device}
sudo rmmod $module.ko
dmesg
sudo dmesg -C
exit 0