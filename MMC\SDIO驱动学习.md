# Linxu MMC 驱动子系统

## 硬件关联

CPU、MMC controller、存储设备之间的关联如下图所示，主要包括了MMC controller、总线、存储卡等内容的连接，针对控制器与设备的总线连接，主要包括时钟、数据、命令三种类型的引脚，而这些引脚中的cd引脚主要用于卡的在位检测，当mmc controller检测到该位的变化后，则会进行mmc card的注册或注销操作。

![cpu-mmc](./resource/cpu%20mmc硬件关联图.jpeg)

## 目录说明

针对mmc子系统，在代码实现上主要包括mmc core、mmc block、 mmc host这三个模块

- mmc host主要是针对mmc controller的驱动；
- mmc block主要用于实现mmc block驱动以及mmc driver；
- 而mmc core主要包括mmc 总线、sdio总线的实现、mmc device、mmc driver的注册接口、mmc host与mmc card的注册与注销接口等内容。

## 系统层次

MMC子系统从上到下分为3层

- **块设备层**(MMC card)：与Linux的块设备子系统对接，实现块设备驱动以及完成请求，负责驱动MMC core抽象出来的虚拟的card设备，如mmc、sd、tf卡。

- **核心层**(MMC core)：是不同协议和规范的实现，为MMC控制器层和块设备驱动层提供接口函数。
  
  核心层封装了 MMC/SD 卡的命令（CMD)，例如存储卡的识别、设置、读写、识别、设置等命令。

  MMC核心层由三个部分组成：MMC，SD和SDIO，分别为三类设备驱动提供接口函数；

  core.c 把 MMC 卡、 SD 卡的共性抽象出来，它们的差别由 sd.c 和 sd_ops.c 、 mmc.c 和 mmc_ops.c 来完成。

- **控制器层**(MMC host)：主机端MMC controller的驱动，依赖于平台。由struct mmc_host描述,围绕此结构设计了struct mmc_host_ops(访问方法)、struct mmc_ios(相关参数)、struct mmc_bus_ops(电源管理和在位检测方法)
  
  针对不同芯片，实现不同控制器对应的驱动代码。

![框架结构](resource/mmc子系统框架结构图.jpeg)

块设备层与Linux的块设备子系统对接，实现块设备驱动以及完成请求，具体协议经过核心层的接口，最终通过控制器层完成传输，对MMC设备进行实际的操作。

## MMC驱动抽象模型

MMC驱动模型也是基于实际的硬件连接进行抽象的

- 针对mmc controller，该子系统抽象为**mmc_host**，用于描述一个进行设备通信的控制器，提供了相应的访问接口（记为mmc_host->request）；
- 针对mmc、sd、tf卡具体设备，该子系统抽象为**mmc_card**，用于描述卡信息。mmc子系统提供年rescan接口用于mmc card的注册；
- 针对通信总线，抽象出**mmc_bus**；
- 针对mmc、sd、tf，mmc子系统完成了统一的**mmc driver**，针对mmc总线规范以及SD规范，其已经详细的定义了一个存储卡的通信方式、通信命令，因此LINUXmmc子系统定义了mmc driver，用于和mmc、sd、tf等卡的通信，而**不需要**驱动开发人员来开发卡驱动。

![bus dribber host card](resource/bus%20driver%20host%20card%20关联图.jpeg)

特点:

1. mmc总线模型仅注册一个驱动类型，即mmc driver
2. 一个mmc host与一个mmc card绑定
3. mmc card属于热插拔的设备，而mmc card的创建主要由mmc host负责探测与创建，mmc host根据卡在位检测引脚，当检测到mmc card的存在后，即创建mmc card，同时注册至mmc bus上，并完成与mmc driver的绑定操作。
4. host和card可以分别理解为 MMC device的两个子设备：MMC主设备和MMC从设备，其中host为集成于MMC设备内部的MMC controller，card为MMC设备内部实际的存储设备。

## SDIO驱动抽象模型

sdio总线驱动模型和mmc类似，结构体上的区别为其driver类型为sdio_driver，并增加了sdio_func结构体变量（该结构体包含了该sdio设备相关的厂商id、设备id，同时包含了mmc_card）

因sdio主要突出接口概念，其设备端可以连接wifi、gps等设备，因此其外设备驱动**需要**由驱动工程师自己实现，sdio驱动模块不提供对应的驱动。

## 设备-总线-驱动模型

针对MMC子系统而言，主要使用了系统中的两个模型：**设备-总线-驱动模型**、**块设备驱动**模型。

在Linux驱动模型框架下，MMC驱动子系统的对应关系如下：

- 总线 bus_type ———— MMC总线（ mmc_bus )
- 设备 device ———— 被封装在platform_device下的主设备**host**
- 驱动 device_driver ———— 依附于MMC总线的MMC驱动( mmc_driver )
  
设备和对应的驱动必须依附于同一种总线

## MMC总线注册

调用入口位于mmc/core/core.c，通过mmc_intit()实现。

主要包括两个方面：

- 利用 bus_register() 注册 mmc_bus，包括mmc总线和sdio总线。对应sysfs下的 /sys/bus/mmc/ 目录。
- 利用 class_register() 注册 mmc_host_class 。对应sysfs下的 /sys/class/mmc_host 目录。

关键函数为：```c bus_register()```

## MMC驱动注册

调用入口位于mmc/core/block.c,将mmc_driver注册到mmc_bus总线中.

主要步骤包括：

- 通过 register_blkdev() 向内核注册块设备。
  
  借助该块设备驱动模型，将mmc card与vfs完成了关联，即可通过系统调用借助VFS模型实现对块设备的读写访问操作。

- 调用 driver_register() 将 mmc_driver 注册到 mmc_bus 总线系统。和其他驱动注册方式一致。

关键函数为：```c platform_driver_register()  -->  driver_register()  -->  bus_add_driver()```

## MMC设备注册

每个host均有调用入口,使用module_platform_driver()宏实现。

驱动入口函数中将注册 platform_driver 和 platform_device ， name 均定义为 xxx_mmc 。根据驱动模型，最终会回调 xxx_mmc_driver 中的 probe() 函数： xxx_mmc_probe() 。

关键函数为: ```cplatform_device_add()  -->  device_add()  -->  bus_add_device()  /  bus_probe_device()```

## 注册过程(瑞芯微MMC驱动源码)

设备启动时，首先向linux系统注册mmc_bus和sdio_bus两条总线,用来管理块设备和sdio接口类型的设备。同时注册mmc_host_class类

```c
# core.c
subsys_initcall(mmc_init);
```

接着调用module_init向系统注册一条mmc_rpmb_bus总线、一个mmc块设备和mmc driver。

```c
# block.c
module_init(mmc_blk_init);
```

最后调用module_platform_driver，把mmc controler注册到platform总线，同时扫描一次挂载到mmc控制器上的设备。

```c
# meson-mx-sdio.c
module_platform_driver(mmc_pwrseq_emmc_driver);
```

## 调用过程
