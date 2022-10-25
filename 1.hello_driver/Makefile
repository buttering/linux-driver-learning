obj-m += hello_driver.o
# 说明有一个模块需要从main.o中构造，构造的模块名称为xxx.ko

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	# -C 指定内核源代码树位置，M=选项让该makefile在构造modules目标之前返回到模块源码目录，modules目标指向obj-m变量制定的模块
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	sudo dmesg -C
	sudo insmod hello_driver.ko howmany=10 whom="Mom"

	sudo rmmod hello_driver.ko
	dmesg