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
    # /dev目录下的chrdev
    device_name="async"
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