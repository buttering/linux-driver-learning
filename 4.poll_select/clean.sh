#!/bin/zsh


if [ $# -ne 1 ]
then 
    echo "\033[31mneed arg!\033[31m"
    exit 1
fi
test_file_name=$1
if [ $test_file_name = "poll_select" ]
then
    module="poll_select"
    device="polldev"
    testfile="test_poll"
else    
    exit 1
fi

if test -z $device
then
    echo "\033[31mgdevice is empty!\033[31m"
    exit 1
fi

sudo dmesg -C
sudo rm -r /dev/${device}
sudo rmmod $module.ko
dmesg
sudo dmesg -C

rm $testfile
exit 0