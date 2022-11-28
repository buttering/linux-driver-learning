#!/bin/zsh


if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit 1
fi
test_file_name=$1
if [ $test_file_name = "async" ]
then
    module_name="async"
    device_name="asyncdevice"
    testfile=""
else    
    exit 1
fi

if test -z $device_name
then
    echo "\033[31mdevice is empty!\033[31m"
    exit 1
fi

sudo dmesg -C
sudo rm -r /dev/${device_name}
sudo rmmod $module_name.ko
dmesg
sudo dmesg -C

exit 0