#!/bin/zsh


if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit 1
fi
driver_file_name=$1
if [ $driver_file_name = "async" ]
then
    module_name="async"
    device_name="asyncdevice"
    testfile_name="test_async"
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

rm $testfile_name
exit 0