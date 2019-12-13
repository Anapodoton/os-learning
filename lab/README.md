<!--ts-->
   * [操作系统实验概述](#操作系统实验概述)
      * [实验的基础应是可实际运行的操作系统](#实验的基础应是可实际运行的操作系统)
      * [基本内容](#基本内容)
   * [3. 实验环境的搭建和使用](#3-实验环境的搭建和使用)
      * [主要平台和工具简介](#主要平台和工具简介)
         * [x86模拟器Bochs](#x86模拟器bochs)
         * [GCC编译器](#gcc编译器)
         * [GDB调试器](#gdb调试器)
         * [Ubuntu (GNU/Linux)](#ubuntu-gnulinux)
         * [Microsoft Windows](#microsoft-windows)
         * [源代码阅读工具](#源代码阅读工具)
         * [CodeView-oslab集成环境](#codeview-oslab集成环境)
      * [实验环境的工作模式](#实验环境的工作模式)
         * [环境搭建](#环境搭建)
         * [Ubuntu下环境搭建](#ubuntu下环境搭建)
         * [使用方法](#使用方法)
         * [准备活动](#准备活动)
         * [编译内核](#编译内核)
         * [运行和调试](#运行和调试)
         * [Ubuntu和Linux 0.11之间的文件交换](#ubuntu和linux-011之间的文件交换)
      * [Windows上的实验环境](#windows上的实验环境)
         * [环境搭建](#环境搭建-1)
         * [使用方法](#使用方法-1)
         * [编译内核](#编译内核-1)
         * [运行和调试](#运行和调试-1)
         * [Windows和Linux 0.11之间的文件交换](#windows和linux-011之间的文件交换)
      * [FAQ](#faq)
   * [4 操作系统的引导](#4-操作系统的引导)
      * [实验目的](#实验目的)
      * [实验内容](#实验内容)
      * [实验报告](#实验报告)
      * [实验提示](#实验提示)
      * [Linux 0.11相关代码详解](#linux-011相关代码详解)
         * [引导程序的运行环境](#引导程序的运行环境)
         * [完成bootsect.s的屏幕输出功能](#完成bootsects的屏幕输出功能)
         * [编译和运行](#编译和运行)
         * [bootsect.s读入setup.s](#bootsects读入setups)
         * [再次编译](#再次编译)
         * [setup.s获取基本硬件参数](#setups获取基本硬件参数)
   * [5. 系统调用](#5-系统调用)
      * [实验目的](#实验目的-1)
      * [实验内容](#实验内容-1)
         * [iam()](#iam)
         * [whoami()](#whoami)
         * [测试程序](#测试程序)
      * [实验报告](#实验报告-1)
      * [评分标准](#评分标准)
      * [实验提示](#实验提示-1)
         * [应用程序如何调用系统调用](#应用程序如何调用系统调用)
         * [从“int 0x80”进入内核函数](#从int-0x80进入内核函数)
      * [实现sys_iam()和sys_whoami()](#实现sys_iam和sys_whoami)
      * [修改Makefile](#修改makefile)
         * [用printk()调试内核](#用printk调试内核)
         * [编写测试程序](#编写测试程序)
         * [在用户态和核心态之间传递数据](#在用户态和核心态之间传递数据)
         * [运行脚本程序](#运行脚本程序)
         * [errno](#errno)
   * [6. 进程运行轨迹的跟踪与统计](#6-进程运行轨迹的跟踪与统计)
      * [实验目的](#实验目的-2)
      * [实验内容](#实验内容-2)
      * [实验报告](#实验报告-2)
      * [评分标准](#评分标准-1)
      * [实验提示](#实验提示-2)
         * [编写样本程序](#编写样本程序)
            * [小技巧：](#小技巧)
         * [log文件](#log文件)
            * [打开log文件](#打开log文件)
         * [写log文件](#写log文件)
         * [跟踪进程运行轨迹](#跟踪进程运行轨迹)
         * [jiffies，滴答](#jiffies滴答)
         * [寻找状态切换点](#寻找状态切换点)
         * [管理log文件](#管理log文件)
         * [数据统计](#数据统计)
         * [修改时间片](#修改时间片)
   * [7. 基于内核栈切换的进程切换](#7-基于内核栈切换的进程切换)
      * [课程说明](#课程说明)
      * [实验目的](#实验目的-3)
      * [实验内容](#实验内容-3)
      * [实验报告](#实验报告-3)
            * [问题 1](#问题-1)
            * [问题 2](#问题-2)
            * [问题 3](#问题-3)
      * [实验提示](#实验提示-3)
         * [TSS 切换](#tss-切换)
         * [本次实验的内容](#本次实验的内容)
         * [schedule 与 switch_to](#schedule-与-switch_to)
         * [实现 switch_to](#实现-switch_to)
         * [修改 fork](#修改-fork)

<!-- Added by: anapodoton, at: 2019年12月 9日 星期一 16时22分05秒 CST -->

<!--te-->

# 操作系统实验概述

实践对于操作系统课程是至关重要的，但这种实践决不应该仅仅停留在某些算法的模拟实现上。就算在编程实现了一个完整的“文件系统”，但如果没有和磁盘驱动、高速缓存、内存管理等模块联系在一起，即使编写了大量的代码，也只是实现了某些概念（更象是一个数据结构课程设计），仍然不能控制可实际运行的操作系统。本实验指导书强调在可实际运行的操作系统上的实践。

## 实验的基础应是可实际运行的操作系统

因此本操作系统实验指导书上要求的实验以实际可运行的操作系统为基础，就选取实际操作系统。可供选择的操作系统有很多，如Windows、Linux、UNIX、VxWorks等，对于第一次学习操作系统的本科生来说，应该选取他们有过使用经验的操作系统为例，这样才能充分激发学生的学习兴趣(深入探秘身边的事物是人们都有的好奇心)。由于PC上Windows使用是非常广泛的，所以Windows是一个不错的选择。但另一方面，Windows关于内核的资料太少(属商业机密)，虽然微软也开发了部分源码，但读懂这些源码并形成文档资料的人还很少。相比而言，Linux操作系统由于完全开源，资料非常多，被越来越多的人，尤其是大学生所使用，所以Linux是一个非常不错的选择。

在Linux上也有两种选择，一种是选择最新的Linux版本，另一种是选择一个简单的早期版本。新的成熟的Linux版本(如Linux 2.6.11)虽然非常成熟，但代码量也非常巨大，达到500多万行，代码也非常复杂，其上的改动、编译会非常困难、缓慢，不适合作为本科操作系统课程的实验课。所以考虑到第一次接触操作系统课程时所需要的简单性、参考资料应该容易获取、以及将来可进行进一步的挖掘，本指导书选取在Linux 0.11上进行操作系统的课程实验。Linux 0.11内核有不到2万行的代码，容易读懂，修改容易(在课堂上已有深入的分析)，但Linux 0.11也是一个完整的、在应用和Linux没有太多区别的系统，其上的时间能使学生得到充分的锻炼，为理解操作系统、为自己编写操作系统奠定了坚实的基础。

## 基本内容

本实验指导书要求学生在Linux 0.11基础上完成8个操作系统实验，并将这些实验的结果放在Bochs中执行，获得实验结果。

8个验证型实验包括：系统引导、系统调用、进程运行轨迹的跟踪、信号量实现和应用、Linux 0.11地址映射跟踪、终端显示的控制、实现一个类似现在Linux上的proc文件系统(通过访问proc文件来了解内核情况，如CPU类型、内存使用、进程情况等)；在Linux 0.11上实现交换分区和全局页面置换策略。下表(表1-1)给出了更清晰的描述。

|  编号  |                 实验项目                 |                           实验内容                           |                           实验目的                           |
| :----: | :--------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| 实验一 |                 系统引导                 |      编写bootsect.s和setup.s，实现一个简单的系统引导。       |       深入认识系统引导过程，掌握操作系统开发基本过程。       |
| 实验二 |                 系统调用                 |                   添加一个系统调用whoami。                   |               深入认识操作系统接口的工作原理。               |
| 实验三 |            进程运行轨迹的跟踪            | 编写程序跟踪进程的运行轨迹，主要是进程的状态切换过程。同时替换现有的Linux 0.11进程调度算法，并和现有的进程调度进行对比。 |       深入认识进程，能通过实际试验来评价操作系统性能。       |
| 实验四 |             信号量实现和应用             |      在Linux 0.11上实现信号量，并实践Multiprogramming。      |          体会并发进程，掌握进程同步(互斥)技术的实现          |
| 实验五 |       Linux 0.11地址映射跟踪与共享       |          打印Linux 0.11的地址映射过程到log文件中。           |       实践逻辑地址、线性地址、物理地址、段页表等概念。       |
| 实验六 |               控制终端显示               |     特殊按键修改系统状态，并控制显示(如按F1后总显示**)。     |          深入认识设备管理，掌握终端设备驱动的实现。          |
| 实验七 |            proc文件系统的实现            |      在Linux 0.11上实现一个proc文件系统，动态察看内核。      |        体会虚拟文件系统概念，掌握文件系统的实现过程。        |
| 实验八 | 在Linux 0.11上实现交换分区和全局页面置换 | 在Linux 0.11上完整的虚拟内存，主要是实现全局的页面置换和交换分区设备的管理，并将这两个模块放入系统。 | 相比前面的实验而言，本实验较难。将全面加深进程、虚存、磁盘、中断的认识，对OS的系统性获得直接经验。 |

表1-1 本指导书要求的实验内容概述

可以看出，本指导书要求的实验覆盖了进程管理、内存管理、设备管理、文件系统、系统引导和系统接口等操作系统的基本内容，认真地完成本指导书所要求的实验内容后，相信读者会对操作系统建立全新的、深入的、美妙的认识，也必将树立更强的信心，可以完成和操作系统有关的各种软件开发任务，甚至是设计一个新的操作系统。



本操作系统实验的硬件环境是IA-32(x86)架构的PC机（就是你现在正在使用的计算机），主要软件环境是Bochs + gcc + 你最喜欢的编辑器/IDE + 你最喜欢的操作系统 + Linux 0.11源代码。实验的基本流程是根据实验要求编写应用程序、修改Linux 0.11的源代码，用gcc编译后，在Bochs的虚拟环境中运行、调试目标代码。

上述实验环境涉及到的软件都是免费且开源的，具有较强的可移植性，可以在多种计算机的多种操作系统上搭建。为方便实验者，我们在最常见的两个平台——Windows和Ubuntu（最流行的GNU/Linux发行版之一）——上制作了hit-oslab集成环境。它基本包含了实验所需的所有软件，安装过程非常简单。对于其他平台，可参考集成环境搭建实验环境，本书不做介绍。

# 3. 实验环境的搭建和使用

## 主要平台和工具简介

### x86模拟器Bochs

Bochs是一个免费且开放源代码的IA-32(x86)架构PC机模拟器。在它模拟出的环境中可以运行Linux、DOS和各种版本的Windows等多种操作系统。而Bochs本身具有很高的移植性，可以运行在多种软硬件平台之上，这也是我们选择它做为本书的指定模拟器的主要原因。如果您想拥抱自由的Linux，那么Bochs几乎是您的不二选择。如果您想继续把自己绑定在Windows平台上，那么除了Bochs，您还可以选用VMware或者Microsoft Virtual PC。它们是最著名虚拟机软件，而且都可以免费使用。因为Bochs的是模拟器，其原理决定了它的运行效率会低于虚拟机。但对于本书所设计的实验来说，效率上的差别很不明显。而且，Bochs有虚拟机无可比拟的调试操作系统的能力，所以我们更建议您选用Bochs。hit-oslab已经内置了bochs，本书后文假定的缺省环境也是Bochs。

关于Bochs的更详细的介绍请访问它的主页及[Bochs使用手册](https://hoverwinter.gitbooks.io/hit-oslab-manual/content/bochs-manual.html)。

### GCC编译器

GCC是和Linux一起成长起来的编译器。Linux最初的版本就是由GCC编译的。现在GCC也是在自由软件领域应用最广泛的编译器。所以，我们也选择GCC做为本书实验的指定编译器。

### GDB调试器

GDB调试器是GCC编译器的兄弟。做为自由软件领域几乎是唯一的调试器，它秉承了*nix类操作系统的一贯风格，采用纯命令行操作，有点儿类似dos下的debug。关于它的使用方法请看GDB使用手册。

### Ubuntu (GNU/Linux)

Ubuntu也许不是目前最好用的Linux桌面发行版，但它一定是最流行的。主要特点是易用，非常的易用。

现在，已经有越来越多的人开始用Ubuntu完全代替Windows，享受更加自由、安全、守法的感觉。Ubuntu的主页是http://www.ubuntu.com/ ，这里不仅可以免费下载到iso文件，甚至能免费申领Ubuntu的安装光盘。

我们强烈建议您在Ubuntu下做实验。因为有些实验内容涉及到在自己改进的Linux 0.11下，运行自己编的应用程序。被改进的功能都是高版本Linux内核已经具有的，在其上确认自己编写的应用程序无误后，再用之测试自己改进的Linux 0.11，可以更有信心些。

### Microsoft Windows

人类历史上最重要的操作系统之一，用户最多的操作系统（没有之一），天下谁人不识君。

### 源代码阅读工具

实验过程中不可避免地要阅读Linux 0.11的源代码。源代码阅读工具能让这个过程的效率更高，烦恼更少。

http://www.oldlinux.org/lxr/http/source/ 是用lxr生成的一个Linux源代码阅读网站。只要用鼠标点击，就能轻松浏览Linux 0.11的源代码。

但在不能上网的时候，或者阅读的同时还要修改源代码，就要使用下面介绍的工具了。

在Linux下，ctags可以和VIM或Emacs配合，一边浏览代码，一边编辑。在Ubuntu下这样安装：

$ sudo apt-get install exuberant-ctags

使用起来也不麻烦，网上资料很多，man文档也很详细。

在Windows下，Source Insight是个不错的选择。

### CodeView-oslab集成环境

0603102班的高汉东同学在使用hit-oslab后，对它深恶痛绝，他是这么说的：

“……在阅读linux-0.11源代码的的过程中，我们经常遇到一个问题：在一个文件中调用的函数并不在这个文件中定义，即所调用的函数是一个外部函数。为了找到这个函数的定义，我们要不就翻遍文件夹内所有的文件，要不就在“内核注释”中搜索。这两种方法都可以解决，但是很麻烦。……”

于是，他伙同几名同学，开发了CodeView，并和oslab深度集成。该软件为开源软件，支持Windows和Linux，不仅欢迎下载，而且欢迎有识之士提交建议、补丁，甚至加盟。

## 实验环境的工作模式

hit-oslab实验环境简称oslab，可在“[资料和文件](https://github.com/traitorousfc/HIT-OSLab/tree/master/Resources)”下载。oslab工作在一个宿主操作系统之上，分为Linux版和Win32版。其中Linux版主要针对Ubuntu的32位和64位版设计，但稍加修改甚至完全不修改，也可以在其它Linux发行版使用。这是我们主要推荐的版本。Win32版主要针对Windows XP设计，不能很好地支持Windows Vista。

在宿主操作系统之上完成对Linux 0.11的开发、修改和编译之后，在linux-0.11目录下会生产一个名为Image的文件，它就是编译之后的目标文件。该文件内已经包含引导和所有内核的二进制代码。如果拿来一张软盘，从它的0扇区开始，逐字节写入Image文件的内容，就可以用这张软盘启动一台真正的计算机，并进入Linux 0.11内核。oslab采用bochs模拟器加载这个文件，模拟执行Linux 0.11，这样省却了重新启动计算机的麻烦。

bochs目录下是与bochs相关的执行文件、数据文件和配置文件。run是运行bochs的脚本命令。运行后bochs会自动在它的虚拟软驱A和虚拟硬盘上各挂载一个镜像文件，软驱上挂载是linux-0.11/Image，硬盘上挂载的是hdc-0.11.img。因为bochs配置文件中的设置是从软驱A启动，所以Linux 0.11会被自动加载。而Linux 0.11会驱动硬盘，并mount硬盘上的文件系统，也就是将hdc-0.11.img内镜像的文件系统挂载到0.11系统内的根目录——“/”。在0.11下访问文件系统，访问的就是hdc-0.11.img文件内虚拟的文件系统。

hdc-0.11.img文件的格式是Minix文件系统的镜像。Linux所有版本都支持这种格式的文件系统，所以可以直接在宿主Linux上通过mount命令访问此文件内的文件，达到宿主系统和bochs内运行的Linux 0.11之间交换文件的效果。Windows下目前没有（或者是还没发现）直接访问Minix文件系统的办法，所以要借助于fdb.img，这是一个1.44M软盘的镜像文件，内部是FAT12文件系统。将它挂载到bochs的软驱B，就可以在0.11中访问它。而通过filedisk或者WinImage，可以在Windows下访问它内部的文件。

hdc-0.11.img内包含有：

- Bash shell
- 一些基本的Linux命令、工具，比如cp、rm、mv、tar。
- vi编辑器
- gcc 1.4编译器，可用来编译标准C程序
- as86和ld86
- Linux 0.11的源代码，可在0.11下编译，然后覆盖现有的二进制内核
- Linux上的实验环境[编辑]

感谢兰州大学的falcon，他写了一篇详细的文章介绍如何在Linux下搭建Linux 0.11的实验环境，并且给出了相应的补丁。本环境大量参考和继承了他所做的工作。

### 环境搭建

### Ubuntu下环境搭建

首先，到“资料和文件”中下载“[hit-oslab-linux-XXXXXXXX.tar.gz](https://github.com/traitorousfc/HIT-OSLab/tree/master/Resources)”。此文件包中包括下列内容：

- 可在Ubuntu下编译的Linux 0.11内核源代码。

- 已打开gdb-stub功能的Bochs及其支撑文件、配置文件。Ubuntu自带的Bochs没有gdb-stub功能，不能用GDB进行C语言级的调试。

- 可忽略Signal 0的GDB调试器。GDB是为调试应用程序而设计，而应用程序不需要处理signal 0，所以GDB捕获到signal 0后会强制暂停程序。Bochs（也许是Linux 0.11内核）会产生大量的signal 0，影响调试。我们给GDB打了一个补丁，使其可以忽略signal 0。

- 磁盘镜像文件hdc-0.11-new.img。它是Linux 0.11的根文件系统，内含gcc、vi等开发工具和bash等常用工具。

- 方便运行、调试和文件交换的一系列脚本。

- 建议将oslab.tar.gz保存到你的home目录(/home/xxxx (xxxx是你的用户名) )下。然后打开终端窗口（左上角的菜单，Applications->Accessories->Terminal），当前目录就是你的home目录。这个窗口是将来要使用的主要窗口。在提示符下执行如下命令（不包括那个“$”）解压缩下载的文件：

  $ tar zxf oslab.tar.gz

用ls命令列目录如果能看到“oslab”目录，就说明解压缩成功。这个目录下已经包括Linux-0.11源代码、Bochs、GDB和一些数据文件及脚本。但我们还需要安装编译器和编辑器等开发环境。

首先安装[gcc-3.4](https://github.com/traitorousfc/HIT-OSLab/tree/master/Resources)。因为Linux-0.11不能在gcc 4.x版本编译，所以要装老一点儿的编译器。在Ubuntu 9.04(jaunty)及之前，用下面命令安装：

```
$ sudo apt-get install gcc-3.4
```

在Ubuntu 9.10(karmic)及之后，要先从“资料和文件”中下载gcc-3.4-ubuntu.tar.gz到/tmp目录，用下列命令安装：

```
$ cd /tmp
$ tar zxvf gcc-3.4-ubuntu.tar.gz
$ cd gcc-3.4
$ sudo ./inst.sh xxxx #xxxx换为i386或amd64
```

然后执行下列命令确保其它必备工具已安装：

```
$ sudo apt-get install build-essential bin86 manpages-dev
```

如果使用的是64位的系统，还要：

```
$ sudo apt-get install libc6-dev-i386 ia32-libs ia32-libs-gtk
```

还要安装你喜欢（或将来会喜欢）的编辑器/IDE。我们推荐vim：

```
$ sudo apt-get install vim cscope exuberant-ctags
```

也推荐emacs：

```
$ sudo apt-get install emacs
```

“vimtutor”是一个非常简单直接的vim入门培训命令，安装了vim后，它就已经存在，可以试试。

Linux上肯定也有类似Windows的编辑器，但并不推荐它们，因为真正的Linuxer只喜欢vi或者emacs。

选择了一种编辑器，就选择了一种文化和生活习惯，所以请慎重选择。

至此，环境搭建完毕。

Fedora下环境搭建

本节由计算机学院09级杨靖同学撰写。

首先，请参考Ubuntu实验环境的搭建，下载并解压hit-oslab包。

之后是安装必要的工具：gcc的3.4版本，因为linux-0.11不能在gcc 4.x下编译。 在Fedora下，用下列命令安装：

```
$sudo yum install compat-gcc-34 glibc-devel libgcc
```

如果没有sudo权限，则运行

```
$su -c "yum install compat-gcc-34 glibc-devel libgcc"
```

以下命令类似，如果不能sudo则su。su命令需要root密码，（应该）是在安装Fedora的时候指定的。

如果是64位系统，那么应该指定安装32位的库。 在64位Fedora下运行如下命令来安装gcc 3.4

```
$sudo yum install compat-gcc-34 glibc-devel.i686 libgcc.i686
```

安装完成之后再以普通用户身份运行

```
$ln -s /usr/bin/gcc34 ~/bin/gcc-3.4
```

然后是其他必备工具

```
$sudo yum install dev86
```

还有一些必要的库，

如果是32位系统

```
$sudo yum install libSM libX11 libXpm libstdc++ ncurses-libs expat
```

如果是64位的系统

```
$sudo yum install libSM.i686 libX11.i686 libXpm.i686 libstdc++.i686 ncurses-libs.i686 expat.i686
```

即使安装了这些软件包，实验时还是有可能出现找不到某些lib的情况。

遇到这种情况请先读错误输出，从中找出缺少的库（一般都是libxxx.so.xxx），然后尝试

输入yum search 缺少库的名字( 例如libXpm则输入Xpm）

1. Google “fedora libxxx”
2. 到cms的论坛上发帖求助
3. 这三种方法来找到所需的软件包并安装上。

另外，在Fedora与linux-0.11交换文件时，可能需要暂时关闭SElinux，否则SElinux会阻止用户的某些磁盘写入操作。

### 使用方法

### 准备活动

```
$ cd ~/oslab
```

把当前目录切换到oslab下，用pwd命令确认，用“ls -l”列目录内容。本实验的所有内容都在本目录或其下级目录内完成。

### 编译内核

“编译内核”比“编写内核”要简单得多。首先要进入linux-0.11目录，然后执行：

```
$ make all
```

因为“all”是最常用的参数，所以可以省略，只用“make”，效果一样。

在多处理器的系统上，可以用-j参数进行并行编译，加快速度。例如双CPU的系统可以：

```
$ make -j 2
```

make命令会显示很多很多很多的信息，你可以尽量去看懂，也可以装作没看见。只要最后几行中没有“error”就说明编译成功。最后生成的目标文件是一个软盘镜像文件——linux-0.11/Image。如果将此镜像文件写到一张1.44MB的软盘上，就可以启动一台真正的计算机。

linux-0.11目录下是全部的源代码，很多实验内容都是要靠修改这些代码来完成。修改后需要重新编译内核，还是执行命令：

```
$ make all
```

make命令会自动跳过未被修改的文件，链接时直接使用上次编译生成的目标文件，从而节约编译时间。但如果重新编译后，你的修改貌似没有生效，可以试试先“make clean”，再“make all”。“make clean”是删除上一次编译生成的所有中间文件和目标文件，确保是在全新的状态下编译整个工程。

### 运行和调试

在Bochs中运行最新编译好的内核很简单，在oslab目录下执行：

```
$ ./run
```

如果出现Bochs的窗口，里面显示linux的引导过程，最后停止在“[/usr/root/]#”，表示运行成功。

内核调试分为两种模式：汇编级调试和C语言级调试。

汇编级调试需要执行命令：

```
$ ./dbg-asm
```

可以用命令help来查看调试系统用的基本命令。更详细的信息请查阅Bochs使用手册。

C语言级调试稍微复杂一些。首先执行如下命令：

```
$ ./dbg-c
```

然后再打开一个终端窗口，进入oslab目录后，执行：

```
$ ./rungdb
```

新终端窗口中运行的是GDB调试器。关于gdb调试器请查阅GDB使用手册。

### Ubuntu和Linux 0.11之间的文件交换

oslab下的hdc-0.11-new.img是0.11内核启动后的根文件系统镜像文件，相当于在bochs虚拟机里装载的硬盘。在Ubuntu上访问其内容的方法是：

```
$ sudo ./mount-hdc
```

（在格物楼机房，直接用“mount hdc”代替上面命令，不需sudo。但要求oslab必须在/home/public_user/oslab或/public/oslab）

之后，hdc目录下就是和0.11内核一模一样的文件系统了，可以读写任何文件（可能有些文件要用sudo才能访问）。读写完毕，不要忘了卸载这个文件系统：

```
$ sudo umount hdc
```

**注意1**：不要在0.11内核运行的时候mount镜像文件，否则可能会损坏文件系统。同理，也不要在已经mount的时候运行0.11内核。

**注意2**：在关闭Bochs之前，需要先在0.11的命令行运行“sync”，确保所有缓存数据都存盘后，再关闭Bochs。

## Windows上的实验环境

（从2008年9月起，Windows版的hit-oslab停止维护和支持。使用中遇到的问题请自行解决。我们不对该版本造成的任何后果负责）

感谢flyfish，他写了一篇详细的文章介绍如何在Windows下搭建Linux 0.11的实验环境，并且给出了相应的代码。本环境大量参考和继承了他所做的工作。

### 环境搭建

首先，到“资料和文件”中下载最新版的hit-oslab-win32-XXXXXXXX.tar.gz。这个文件包中包括下列内容：

- 可在Windows下编译的Linux 0.11内核源代码。
- BIN86、GDB和GCC(MinGW)。
- 已打开gdb-stub功能，且不会发出signal 0的Bochs及其支撑文件、配置文件。Bochs的二进制发行版没有gdb-stub功能，不能用GDB进行C语言级的调试。而GDB和Bochs协同进行远程调试的时候，Bochs会发出大量的signal 0，影响调试。根据roy_hu在http://www.oldlinux.org/oldlinux/viewthread.php?tid=10761所述，我们给Bochs打了一个补丁，解决了此问题。
- 硬盘镜像文件hdc-0.11-new.img。它是Linux 0.11的根文件系统，内含gcc、vi等开发工具和bash等常用工具。
- 软盘镜像文件fdb.img。用来在Windows和Linux 0.11之间交换文件。
- filedisk，在Windows上将fdb.img虚拟成为一个可直接访问的磁盘。
- 方便运行、调试的一系列批处理文件。
- 将oslab解压后，进入oslab文件夹，可以看到文件夹下已经包括Linux-0.11源代码、Bochs、MinGW和一些数据文件及批处理文件。

为了能在Windows NT/2000/2003/XP和虚拟机中运行的Linux 0.11之间交换文件，需要安装filedisk。首先先进入filedisk文件夹，然后双击“setup”安装filedisk，最后根据提示重新启动系统。

Windows Vista以上用户不需做上一步。因为filedisk不支持Vista。Vista用户可以自行下载、安装商业软件WinImage(http://www.winimage.com/)

重启完毕，只需再安装一个你喜欢的编辑器或IDE，例如：UltraEdit、EditPlus等。请自行选择、下载、安装。

至此，环境搭建完毕。

### 使用方法

### 编译内核

“编译内核”比“编写内核”要简单得多，只需双击“make all”。它运行的时候会显示很多很多很多的信息，你可以尽量去看懂，也可以装作没看见。只要最后几行中没有“error”就说明编译成功。最后生成的目标文件是一个软盘镜像文件——linux-0.11\Image。如果将此镜像文件写到一张1.44MB的软盘上，它就可以用来启动一台真正的计算机。

linux-0.11目录下是全部的源代码，很多实验内容都是要靠修改这些代码来完成。修改后需要重新编译内核，这时最好先双击“clean”，它会删除上一次编译生成的所有中间文件和目标文件，然后再“make all”。

每次编译之前都做一次clean可以避免一些不必要的麻烦，但会增大编译时间。怎么做合适，就要靠经验决定了。

### 运行和调试

在Bochs中运行最新编译好的内核很简单，在oslab文件夹下双击“run”

如果出现Bochs的窗口，里面显示linux的引导过程，最后停止在“[/usr/root/]#”，表示运行成功。

内核调试分为两种模式：汇编级调试和C语言级调试。

汇编级调试需要运行“dbg-asm”。在Console窗口中可以用命令help来查看调试系统用的基本命令。更详细的信息请查阅Bochs使用手册。

C语言级调试稍微复杂一点点。

首先运行“dbg-c”。它会打开网络端口1234，所以如果安装了防火墙一类的软件，会弹出提示，一定要允许这个端口被访问。

然后运行“rungdb”，打开的命令行窗口中运行的就是GDB调试器。关于GDB调试器请查阅GDB使用手册。

### Windows和Linux 0.11之间的文件交换

oslab下的fdb.img是一个1.44MB软盘镜像文件，Bochs启动时会自动挂载到b:。

在Windows NT/2000/2003/XP上我们通过filedisk的虚拟磁盘访问这个文件的内容，它是一款自由软件，免费且开放源代码。不过，它不支持Windows Vista，所以在Windows Vista下只能使用WinImage，它可以直接打开img文件，就像打开一个压缩包一样。但它是商业软件，只有30天的免费试用期。出于版权的考虑，我们没有在实验环境中包含它。

如果使用filedisk，只需双击“mount”，系统会出现一个磁盘“X:”（就像插入一个U盘一样），这个磁盘就是fdb.img的虚拟磁盘，可以在其上读写任何文件。读写完毕，不要忘了双击“umount”，卸载这个文件系统。

在Linux 0.11环境里，输入命令：

```
# mcopy hello.c b:
```

可以将文件hello.c写入文件fdb.img中。而命令：

```
# mcopy b:hello.c .
```

则是从fdb.img拷贝到当前目录。其中mcopy来自Linux/UNIX读写MSDOS文件的一个工具mtools。

如果使用上述命令出现“Reset-floppy called”的错误，那么首先执行一次“mdir a:”，再执行一次“mdir b:”，然后就可以mcopy了。

要特别注意的是，当Bochs运行的时候不能mount，而未umount的时候，Bochs不能访问这个镜像文件。

另外，在关闭Bochs之前，需要先在0.11的命令行运行“sync”，确保所有缓存数据都存盘后，再关闭Bochs。

## FAQ

Q: 为何有时Bochs的光标闪动，却不响应我的输入？

A: 按一下Alt，然后再试试。 如果你习惯用Alt+Tab切换窗口，就肯定会遇到这个问题。原因是在Bochs窗口按下Alt，Bochs会接收到Alt按下的事件，然后将此事件传给Linux 0.11。待再按下Tab时，主机操作系统经判断认定这是一个切换窗口的快捷键，于是直接切换窗口，Tab和Alt抬起的事件都不会再发给Bochs。等切换会Bochs，Linux 0.11此时还处于认为Alt已按下的状态，再按任何按键都被解释成是和Alt组合的按键，所以就“好像”不响应了（按数字键可以看到它的响应）。

Q: 怎样加快make clean、make all的速度？

A: 如果只修改了kernel目录下的文件，删除kernel目录下的kernel.o，然后直接make就行了。其它目录方法类似。

Q: Bochs屏幕乱了怎么办？

A: 这是Linux的终端控制和Bochs虚拟的终端之间配合不好导致的，一般在大量输出信息后，会出现混乱，甚至很像死机。此时按ctrl+l可以缓解一下。最好是用输出重定向功能将输出都重定向到一个文件，然后用vi看。

# 4 操作系统的引导

## 实验目的

- 熟悉hit-oslab实验环境；
- 建立对操作系统引导过程的深入认识；
- 掌握操作系统的基本开发过程；
- 能对操作系统代码进行简单的控制，揭开操作系统的神秘面纱。

## 实验内容

此次实验的基本内容是：

1. 阅读《Linux内核完全注释》的第6章，对计算机和Linux 0.11的引导过程进行初步的了解；
2. 按照下面的要求改写0.11的引导程序bootsect.s
3. 有兴趣同学可以做做进入保护模式前的设置程序setup.s。

改写bootsect.s主要完成如下功能：

1. bootsect.s能在屏幕上打印一段提示信息“XXX is booting...”，其中XXX是你给自己的操作系统起的名字，例如LZJos、Sunix等（可以上论坛上秀秀谁的OS名字最帅，也可以显示一个特色logo，以表示自己操作系统的与众不同。）

改写setup.s主要完成如下功能：

1. bootsect.s能完成setup.s的载入，并跳转到setup.s开始地址执行。而setup.s向屏幕输出一行"Now we are in SETUP"。
2. setup.s能获取至少一个基本的硬件参数（如内存参数、显卡参数、硬盘参数等），将其存放在内存的特定地址，并输出到屏幕上。
3. setup.s不再加载Linux内核，保持上述信息显示在屏幕上即可。

## 实验报告

在实验报告中回答如下问题：

1. 有时，继承传统意味着别手蹩脚。x86计算机为了向下兼容，导致启动过程比较复杂。请找出x86计算机启动过程中，被硬件强制，软件必须遵守的两个“多此一举”的步骤（多找几个也无妨），说说它们为什么多此一举，并设计更简洁的替代方案。 评分标准[编辑]
2. bootsect显示正确，30%
3. bootsect正确读入setup，10%
4. setup获取硬件参数正确，20%
5. setup正确显示硬件参数，20%
6. 实验报告，20%

## 实验提示

操作系统的boot代码有很多，并且大部分是相似的。本实验仿照Linux-0.11/boot目录下的bootsect.s和setup.s，以剪裁它们为主线。当然，如果能完全从头编写，并实现实验所要求的功能，是再好不过了。

同济大学赵炯博士的《Linux内核0.11完全注释（修正版V3.0）》（以后简称《注释》）的第6章是非常有帮助的参考，实验中可能遇到的各种问题，几乎都能找到答案。可以在“资料和文件下载”中下载到该书的电子版。同目录中，校友谢煜波撰写的《操作系统引导探究》也是一份很好的参考。

需要注意的是，oslab中的汇编代码使用as86编译，语法和汇编课上所授稍有不同。

下面将给出一些更具体的“提示”。这些提示并不是实验的一步一步的指导，而是罗列了一些实验中可能遇到的困难，并给予相关提示。它们肯定不会涵盖所有问题，也不保证其中的每个字都对完成实验有帮助。所以，它们更适合在你遇到问题时查阅，而不是当作指南一样地亦步亦趋。本书所有实验的提示都是秉承这个思想编写的。

## Linux 0.11相关代码详解

boot/bootsect.s、boot/setup.s和tools/build.c是本实验会涉及到的源文件。它们的功能详见《注释》的6.2、6.3节和16章。

如果使用Windows下的环境，那么要注意Windows环境里提供的build.c是一个经过修改过的版本。Linus Torvalds的原版是将0.11内核的最终目标代码输出到标准输出，由make程序将数据重定向到Image文件，这在Linux、Unix和Minix等系统下都是非常有效的。但Windows本身的缺陷（也许是特色）决定了在Windows下不能这么做，所以flyfish修改了build.c，将输出直接写入到Image（flyfish是写入到Boot.img文件，我们为了两个环境的一致，也为了最大化地与原始版本保持统一，将其改为Image）文件中。同时为了适应Windows的一些特殊情况，他还做了其它一些小修改。

### 引导程序的运行环境

引导程序由BIOS加载并运行。它活动时，操作系统还不存在，整台计算机的所有资源都由它掌控，而能利用的功能只有BIOS中断调用。

### 完成bootsect.s的屏幕输出功能

首先来看完成屏幕显示的关键代码如下：

```
        ! 首先读入光标位置
        mov    ah,#0x03        
        xor    bh,bh
        int    0x10

        ! 显示字符串“LZJos is running...”
        mov    cx,#25            ! 要显示的字符串长度
        mov    bx,#0x0007        ! page 0, attribute 7 (normal)
        mov    bp,#msg1
        mov    ax,#0x1301        ! write string, move cursor
        int    0x10

    inf_loop:
        jmp    inf_loop        ! 后面都不是正经代码了，得往回跳呀
        ! msg1处放置字符串

    msg1:
        .byte 13,10            ! 换行+回车
        .ascii "LZJos is running..."
        .byte 13,10,13,10            ! 两对换行+回车
        !设置引导扇区标记0xAA55
        .org 510
    boot_flag:
        .word 0xAA55            ! 必须有它，才能引导
```

修改98行的cx的值为msg1的长度，和246行msg1的值。

接下来，将完成屏幕显示的代码在开发环境中编译，并使用linux-0.11/tools/build.c将编译后的目标文件做成Image文件。

### 编译和运行

Ubuntu上先从终端进入~/oslab/linux-0.11/boot/目录。Windows上则先双击快捷方式“MinGW32.bat”，将打开一个命令行窗口，当前目录是oslab，用cd命令进入linux-0.11\boot。无论那种系统，都执行下面两个命令编译和链接bootsect.s：

```
    as86 -0 -a -o bootsect.o bootsect.s
    ld86 -0 -s -o bootsect bootsect.o
```

其中-0（注意：这是数字0，不是字母O）表示生成8086的16位目标程序，-a表示生成与GNU as和ld部分兼容的代码，-s告诉链接器ld86去除最后生成的可执行文件中的符号信息。

如果这两个命令没有任何输出，说明编译与链接都通过了。Ubuntu下用ls -l可列出下面的信息：

```
    -rw--x--x    1  root  root  544  Jul  25  15:07   bootsect
    -rw------    1  root  root  257  Jul  25  15:07   bootsect.o
    -rw------    1  root  root  686  Jul  25  14:28   bootsect.s
```

Windows下用dir可列出下面的信息：

```
    2008-07-28  20:14               544 bootsect
    2008-07-28  20:14               924 bootsect.o
    2008-07-26  20:13             5,059 bootsect.s
```

其中bootsect.o是中间文件。bootsect是编译、链接后的目标文件。

需要留意的文件是bootsect的文件大小是544字节，而引导程序必须要正好占用一个磁盘扇区，即512个字节。造成多了32个字节的原因是ld86产生的是Minix可执行文件格式，这样的可执行文件处理文本段、数据段等部分以外，还包括一个Minix可执行文件头部，它的结构如下：

```
    struct exec {
        unsigned char a_magic[2];  //执行文件魔数
        unsigned char a_flags;
        unsigned char a_cpu;       //CPU标识号
        unsigned char a_hdrlen;    //头部长度，32字节或48字节
        unsigned char a_unused;
        unsigned short a_version;
        long a_text; long a_data; long a_bss; //代码段长度、数据段长度、堆长度
        long a_entry;    //执行入口地址
        long a_total;    //分配的内存总量
        long a_syms;     //符号表大小 
    };
```

算一算：6 char(6字节)+1 short(2字节)+6 long(24字节)=32，正好是32个字节，去掉这32个字节后就可以放入引导扇区了（这是tools/build.c的用途之一）。

对于上面的Minix可执行文件，其a_magic[0]=0x01，a_magic[1]=0x03，a_flags=0x10（可执行文件），a_cpu=0x04（表示Intel i8086/8088，如果是0x17则表示Sun公司的SPARC），所以bootsect文件的头几个字节应该是01 03 10 04。为了验证一下，Ubuntu下用命令“hexdump -C bootsect”可以看到：

```
    00000000  01 03 10 04 20 00 00 00  00 02 00 00 00 00 00 00  |.... ...........|
    00000010  00 00 00 00 00 00 00 00  00 82 00 00 00 00 00 00  |................|
    00000020  b8 c0 07 8e d8 8e c0 b4  03 30 ff cd 10 b9 17 00  |.........0......|
    00000030  bb 07 00 bd 3f 00 b8 01  13 cd 10 b8 00 90 8e c0  |....?...........|
    00000040  ba 00 00 b9 02 00 bb 00  02 b8 04 02 cd 13 73 0a  |..............s.|
    00000050  ba 00 00 b8 00 00 cd 13  eb e1 ea 00 00 20 90 0d  |............. ..|
    00000060  0a 53 75 6e 69 78 20 69  73 20 72 75 6e 6e 69 6e  |.Sunix is runnin|
    00000070  67 21 0d 0a 0d 0a 00 00  00 00 00 00 00 00 00 00  |g!..............|
    00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
    *
    00000210  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
    00000220
```

Windows下用UltraEdit把该文件打开，果然如此。

![图1 用UltraEdit打开文件bootsect](https://hoverwinter.gitbooks.io/hit-oslab-manual/content/assets/bootsect.bmp)

图1 用UltraEdit打开文件bootsect

接下来干什么呢？是的，要去掉这32个字节的文件头部（tools/build.c的功能之一就是这个）！随手编个小的文件读写程序都可以去掉它。不过，懒且聪明的人会在Ubuntu下用命令：

```
$ dd bs=1 if=bootsect of=Image skip=32
```

生成的Image就是去掉文件头的bootsect。

Windows下可以用UltraEdit直接删除（选中这32个字节，然后按Ctrl+X）。

去掉这32个字节后，将生成的文件拷贝到linux-0.11目录下，并一定要命名为“Image”（注意大小写）。然后就“run”吧！

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206133310.png)

图2 bootsect引导后的系统启动情况

### bootsect.s读入setup.s

首先编写一个setup.s，该setup.s可以就直接拷贝前面的bootsect.s（可能还需要简单的调整），然后将其中的显示的信息改为：“Now we are in SETUP”。

接下来需要编写bootsect.s中载入setup.s的关键代码。原版bootsect.s中下面的代码就是做这个的。

```
    load_setup:
        mov    dx,#0x0000        !设置驱动器和磁头(drive 0, head 0): 软盘0磁头
        mov    cx,#0x0002        !设置扇区号和磁道(sector 2, track 0):0磁头、0磁道、2扇区
        mov    bx,#0x0200        !设置读入的内存地址：BOOTSEG+address = 512，偏移512字节
        mov    ax,#0x0200+SETUPLEN    !设置读入的扇区个数(service 2, nr of sectors)，
                        !SETUPLEN是读入的扇区个数，Linux 0.11设置的是4，
                        !我们不需要那么多，我们设置为2
        int    0x13            !应用0x13号BIOS中断读入2个setup.s扇区
        jnc    ok_load_setup        !读入成功，跳转到ok_load_setup: ok - continue
        mov    dx,#0x0000         !软驱、软盘有问题才会执行到这里。我们的镜像文件比它们可靠多了
        mov    ax,#0x0000        !否则复位软驱 reset the diskette
        int    0x13
        jmp    load_setup          !重新循环，再次尝试读取
    ok_load_setup:
        ！接下来要干什么？当然是跳到setup执行。
```

所有需要的功能在原版bootsect.s中都是存在的，我们要做的仅仅是删除那些对我们无用的代码。

具体修改：见lab2/setup.s

### 再次编译

现在有两个文件都要编译、链接。一个个手工编译，效率低下，所以借助Makefile是最佳方式。

在Ubuntu下，进入linux-0.11目录后，使用下面命令（注意大小写）：

```
$ make BootImage
```

Windows下，在命令行方式，进入Linux-0.11目录后，使用同样的命令（不需注意大小写）：

```
make BootImage
```

无论哪种系统，都会看到：

```
    Unable to open 'system'
    make: *** [BootImage] Error 1
```

有Error！这是因为make根据Makefile的指引执行了tools/build.c，它是为生成整个内核的镜像文件而设计的，没考虑我们只需要bootsect.s和setup.s的情况。它在向我们要“系统”的核心代码。为完成实验，接下来给它打个小补丁。

修改build.c

build.c从命令行参数得到bootsect、setup和system内核的文件名，将三者做简单的整理后一起写入Image。其中system是第三个参数（argv[3]）。当“make all”或者“makeall”的时候，这个参数传过来的是正确的文件名，build.c会打开它，将内容写入Image。而“make BootImage”时，传过来的是字符串"none"。所以，改造build.c的思路就是当argv[3]是"none"的时候，只写bootsect和setup，忽略所有与system有关的工作，或者在该写system的位置都写上“0”。

修改工作主要集中在build.c的尾部，请斟酌。**修改build.c，注释掉178和179行即可**。

当按照前一节所讲的编译方法编译成功后，run，就得到了如图3所示的运行结果，和我们想得到的结果完全一样。

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206143302.png)

图3 用修改后的bootsect.s和setup.s进行引导的结果

### setup.s获取基本硬件参数

setup.s将获得硬件参数放在内存的0x90000处。原版setup.s中已经完成了光标位置、内存大小、显存大小、显卡参数、第一和第二硬盘参数的保存。

用ah=#0x03调用0x10中断可以读出光标的位置，用ah=#0x88调用0x15中断可以读出内存的大小。有些硬件参数的获取要稍微复杂一些，如磁盘参数表。在PC机中BIOS设定的中断向量表中int 0x41的中断向量位置(4*0x41 = 0x0000:0x0104)存放的并不是中断程序的地址，而是第一个硬盘的基本参数表。第二个硬盘的基本参数表入口地址存于int 0x46中断向量位置处。每个硬盘参数表有16个字节大小。下表给出了硬盘基本参数表的内容：

表1 磁盘基本参数表

| 位移 | 大小 | 说明         |
| :--- | :--- | :----------- |
| 0x00 | 字   | 柱面数       |
| 0x02 | 字节 | 磁头数       |
| …    | …    | …            |
| 0x0E | 字节 | 每磁道扇区数 |
| 0x0F | 字节 | 保留         |

所以获得磁盘参数的方法就是复制数据。

下面是将硬件参数取出来放在内存0x90000的关键代码。

```
    mov    ax,#INITSEG    
    mov    ds,ax !设置ds=0x9000
    mov    ah,#0x03    !读入光标位置
    xor    bh,bh
    int    0x10        !调用0x10中断
    mov    [0],dx        !将光标位置写入0x90000.

    !读入内存大小位置
    mov    ah,#0x88
    int    0x15
    mov    [2],ax

    !从0x41处拷贝16个字节（磁盘参数表）
    mov    ax,#0x0000
    mov    ds,ax
    lds    si,[4*0x41]
    mov    ax,#INITSEG
    mov    es,ax
    mov    di,#0x0004
    mov    cx,#0x10
    rep            !重复16次
    movsb
```

现在已经将硬件参数（只包括光标位置、内存大小和硬盘参数，其他硬件参数取出的方法基本相同，此处略去）取出来放在了0x90000处，接下来的工作是将这些参数显示在屏幕上。这些参数都是一些无符号整数，所以需要做的主要工作是用汇编程序在屏幕上将这些整数显示出来。

以十六进制方式显示比较简单。这是因为十六进制与二进制有很好的对应关系（每4位二进制数和1位十六进制数存在一一对应关系），显示时只需将原二进制数每4位划成一组，按组求对应的ASCII码送显示器即可。ASCII码与十六进制数字的对应关系为：0x30～0x39对应数字0～9，0x41～0x46对应数字a～f。从数字9到a，其ASCII码间隔了7h，这一点在转换时要特别注意。为使一个十六进制数能按高位到低位依次显示，实际编程中，需对bx中的数每次循环左移一组（4位二进制），然后屏蔽掉当前高12位，对当前余下的4位（即1位十六进制数）求其ASCII码，要判断它是0～9还是a～f，是前者则加0x30得对应的ASCII码，后者则要加0x37才行，最后送显示器输出。以上步骤重复4次，就可以完成bx中数以4位十六进制的形式显示出来。

下面是完成显示16进制数的汇编语言程序的关键代码，其中用到的BIOS中断为INT 0x10，功能号0x0E（显示一个字符），即AH=0x0E，AL=要显示字符的ASCII码。

```
    !以16进制方式打印栈顶的16位数
    print_hex:
        mov    cx,#4         ! 4个十六进制数字
        mov    dx,(bp)     ! 将(bp)所指的值放入dx中，如果bp是指向栈顶的话
    print_digit:
        rol    dx,#4        ! 循环以使低4比特用上 !! 取dx的高4比特移到低4比特处。
        mov    ax,#0xe0f     ! ah = 请求的功能值，al = 半字节(4个比特)掩码。
        and    al,dl         ! 取dl的低4比特值。
        add    al,#0x30     ! 给al数字加上十六进制0x30
        cmp    al,#0x3a
        jl    outp          !是一个不大于十的数字
        add    al,#0x07      !是a～f，要多加7
    outp: 
        int    0x10
        loop    print_digit
        ret
```

这里用到了一个loop指令，每次执行loop指令，cx减1，然后判断cx是否等于0。如果不为0则转移到loop指令后的标号处，实现循环；如果为0顺序执行。另外还有一个非常相似的指令：rep指令，每次执行rep指令，cx减1，然后判断cx是否等于0，如果不为0则继续执行rep指令后的串操作指令，直到cx为0，实现重复。

```
    !打印回车换行
    print_nl:
        mov    ax,#0xe0d     ! CR
        int    0x10
        mov    al,#0xa     ! LF
        int    0x10
        ret
```

只要在适当的位置调用print_bx和print_nl（注意，一定要设置好栈，才能进行函数调用）就能将获得硬件参数打印到屏幕上，完成此次实验的任务。但事情往往并不总是顺利的，前面的两个实验大多数实验者可能一次就编译调试通过了（这里要提醒大家：编写操作系统的代码一定要认真，因为要调试操作系统并不是一件很方便的事）。但在这个实验中会出现运行结果不对的情况（为什么呢？因为我们给的代码并不是100%好用的）。所以接下来要复习一下汇编，并阅读《Bochs使用手册》，学学在Bochs中如何调试操作系统代码。

我想经过漫长而痛苦的调试后，大家一定能兴奋地得到下面的运行结果：

![图4 用可以打印硬件参数的setup.s进行引导的结果](https://hoverwinter.gitbooks.io/hit-oslab-manual/content/assets/hd_figures.bmp)

图4 用可以打印硬件参数的setup.s进行引导的结果

Memory Size是0x3C00KB，算一算刚好是15MB（扩展内存），加上1MB正好是16MB，看看Bochs配置文件bochs/bochsrc.bxrc：

```
……
megs: 16
……
ata0-master: type=disk, mode=flat, cylinders=410, heads=16, spt=38
……
```

这些都和上面打出的参数吻合，表示此次实验是成功的。

# 5. 系统调用

## 实验目的

- 建立对系统调用接口的深入认识
- 掌握系统调用的基本过程
- 能完成系统调用的全面控制
- 为后续实验做准备

## 实验内容

此次实验的基本内容是：在Linux 0.11上添加两个系统调用，并编写两个简单的应用程序测试它们。

### iam()

第一个系统调用是iam()，其原型为：

```
int iam(const char * name);
```

完成的功能是将字符串参数name的内容拷贝到内核中保存下来。要求name的长度不能超过23个字符。返回值是拷贝的字符数。如果name的字符个数超过了23，则返回“-1”，并置errno为EINVAL。

在kernal/who.c中实现此系统调用。

### whoami()

第二个系统调用是whoami()，其原型为：

```
int whoami(char* name, unsigned int size);
```

它将内核中由iam()保存的名字拷贝到name指向的用户地址空间中，同时确保不会对name越界访存（name的大小由size说明）。返回值是拷贝的字符数。如果size小于需要的空间，则返回“-1”，并置errno为EINVAL。

也是在kernal/who.c中实现。

### 测试程序

运行添加过新系统调用的Linux 0.11，在其环境下编写两个测试程序iam.c和whoami.c。最终的运行结果是：

```
$ ./iam lizhijun
$ ./whoami
lizhijun 
```

## 实验报告

在实验报告中回答如下问题：

1. 从Linux 0.11现在的机制看，它的系统调用最多能传递几个参数？你能想出办法来扩大这个限制吗？ 用文字简要描述向Linux 0.11添加一个系统调用foo()的步骤。

## 评分标准

- 将 [testlab2.c](https://github.com/traitorousfc/HIT-OSLab/blob/master/Resources/testlab2.c) 在修改过的Linux 0.11上编译运行，显示的结果即内核程序的得分。满分50%
- 只要至少一个新增的系统调用被成功调用，并且能和用户空间交换参数，可得满分
- 将脚本 [testlab2.sh](https://github.com/traitorousfc/HIT-OSLab/blob/master/Resources/testlab2.sh) 在修改过的Linux 0.11上运行，显示的结果即应用程序的得分。满分30%
- 实验报告，20%

## 实验提示

首先，请将Linux 0.11的源代码恢复到原始状态。

《注释》的5.5节详细讲述了0.11如何处理系统调用，是非常有价值的参考。

操作系统实现系统调用的基本过程是：

1. 应用程序调用库函数（API）；
2. API将系统调用号存入EAX，然后通过中断调用使系统进入内核态；
3. 内核中的中断处理函数根据系统调用号，调用对应的内核函数（系统调用）；
4. 系统调用完成相应功能，将返回值存入EAX，返回到中断处理函数；
5. 中断处理函数返回到API中；
6. API将EAX返回给应用程序。

### 应用程序如何调用系统调用

在通常情况下，调用系统调用和调用一个普通的自定义函数在代码上并没有什么区别，但调用后发生的事情有很大不同。调用自定义函数是通过call指令直接跳转到该函数的地址，继续运行。而调用系统调用，是调用系统库中为该系统调用编写的一个接口函数，叫API（Application Programming Interface）。API并不能完成系统调用的真正功能，它要做的是去调用真正的系统调用，过程是：

- 把系统调用的编号存入EAX
- 把函数参数存入其它通用寄存器
- 触发0x80号中断（int 0x80）

0.11的lib目录下有一些已经实现的API。Linus编写它们的原因是在内核加载完毕后，会切换到用户模式下，做一些初始化工作，然后启动shell。而用户模式下的很多工作需要依赖一些系统调用才能完成，因此在内核中实现了这些系统调用的API。我们不妨看看lib/close.c，研究一下close()的API：

```
#define __LIBRARY__
#include <unistd.h>
_syscall1(int,close,int,fd)
```

其中_syscall1是一个宏，在include/unistd.h中定义。将_syscall1(int,close,int,fd)进行宏展开，可以得到：

```
int close(int fd) 
{ 
    long __res;      
    __asm__ volatile ("int $0x80" 
        : "=a" (__res) 
        : "0" (__NR_close),"b" ((long)(fd)));      
    if (__res >= 0)
        return (int) __res; 
    errno = -__res; 
    return -1; 
}
```

这就是API的定义。它先将宏__NR_close存入EAX，将参数fd存入EBX，然后进行0x80中断调用。调用返回后，从EAX取出返回值，存入__res，再通过对__res的判断决定传给API的调用者什么样的返回值。其中__NR_close就是系统调用的编号，在include/unistd.h中定义：

```
#define __NR_close    6
```

所以添加系统调用时需要修改include/unistd.h文件，使其包含__NR_whoami和__NR_iam。而在应用程序中，要有：

```
#define __LIBRARY__                    /* 有它，_syscall1等才有效。详见unistd.h */
#include <unistd.h>                /* 有它，编译器才能获知自定义的系统调用的编号 */
_syscall1(int, iam, const char*, name);        /* iam()在用户空间的接口函数 */
_syscall2(int, whoami,char*,name,unsigned int,size);    /* whoami()在用户空间的接口函数 */
```

在0.11环境下编译C程序，包含的头文件都在/usr/include目录下。该目录下的unistd.h是标准头文件（它和0.11源码树中的unistd.h并不是同一个文件，虽然内容可能相同），没有__NR_whoami和__NR_iam两个宏，需要手工加上它们，也可以直接从修改过的0.11源码树中拷贝新的unistd.h过来。

### 从“int 0x80”进入内核函数

int 0x80触发后，接下来就是内核的中断处理了。先了解一下0.11处理0x80号中断的过程。

在内核初始化时，主函数（在init/main.c中，Linux实验环境下是main()，Windows下因编译器兼容性问题被换名为start()）调用了sched_init()初始化函数：

```
void main(void)    
{            
    ……
    time_init();
    sched_init();
    buffer_init(buffer_memory_end);
    ……
}
sched_init()在kernel/sched.c中定义为：

void sched_init(void)
{
    ……
    set_system_gate(0x80,&system_call);
}
set_system_gate是个宏，在include/asm/system.h中定义为：

#define set_system_gate(n,addr) \
    _set_gate(&idt[n],15,3,addr)
_set_gate的定义是：

#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
    "movw %0,%%dx\n\t" \
    "movl %%eax,%1\n\t" \
    "movl %%edx,%2" \
    : \
    : "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
    "o" (*((char *) (gate_addr))), \
    "o" (*(4+(char *) (gate_addr))), \
    "d" ((char *) (addr)),"a" (0x00080000))
```

虽然看起来挺麻烦，但实际上很简单，就是填写IDT（中断描述符表），将system_call函数地址写到0x80对应的中断描述符中，也就是在中断0x80发生后，自动调用函数system_call。具体细节请参考《注释》的第4章。

接下来看system_call。该函数纯汇编打造，定义在kernel/system_call.s中：

```
……
nr_system_calls = 72        #这是系统调用总数。如果增删了系统调用，必须做相应修改
……
.globl system_call
.align 2
system_call:
    cmpl $nr_system_calls-1,%eax #检查系统调用编号是否在合法范围内
    ja bad_sys_call
    push %ds
    push %es
    push %fs
    pushl %edx
    pushl %ecx        
    pushl %ebx        # push %ebx,%ecx,%edx，是传递给系统调用的参数
    movl $0x10,%edx        # 让ds,es指向GDT，内核地址空间
    mov %dx,%ds
    mov %dx,%es
    movl $0x17,%edx        # 让fs指向LDT，用户地址空间
    mov %dx,%fs
    call sys_call_table(,%eax,4)
    pushl %eax
    movl current,%eax
    cmpl $0,state(%eax)
    jne reschedule
    cmpl $0,counter(%eax)
    je reschedule
```

system_call用.globl修饰为其他函数可见。Windows实验环境下会看到它有一个下划线前缀，这是不同版本编译器的特质决定的，没有实质区别。call sys_call_table(,%eax,4)之前是一些压栈保护，修改段选择子为内核段，call sys_call_table(,%eax,4)之后是看看是否需要重新调度，这些都与本实验没有直接关系，此处只关心call sys_call_table(,%eax,4)这一句。根据汇编寻址方法它实际上是：

```
call sys_call_table + 4 * %eax   # 其中eax中放的是系统调用号，即__NR_xxxxxx
```

显然，sys_call_table一定是一个函数指针数组的起始地址，它定义在include/linux/sys.h中：

```
fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,……
```

增加实验要求的系统调用，需要在这个函数表中增加两个函数引用——sys_iam和sys_whoami。当然该函数在sys_call_table数组中的位置必须和__NR_xxxxxx的值对应上。同时还要仿照此文件中前面各个系统调用的写法，加上：

```
extern int sys_whoami();
extern int sys_iam();
```

不然，编译会出错的。

## 实现sys_iam()和sys_whoami()

添加系统调用的最后一步，是在内核中实现函数sys_iam()和sys_whoami()。

每个系统调用都有一个sys_xxxxxx()与之对应，它们都是我们学习和模仿的好对象。比如在fs/open.c中的sys_close(int fd)：

```
int sys_close(unsigned int fd)
{
    ……
    return (0);
}
```

它没有什么特别的，都是实实在在地做close()该做的事情。所以只要自己创建一个文件：kernel/who.c，然后实现两个函数就万事大吉了。

## 修改Makefile

要想让我们添加的kernel/who.c可以和其它Linux代码编译链接到一起，必须要修改Makefile文件。Makefile里记录的是所有源程序文件的编译、链接规则，《注释》3.6节有简略介绍。我们之所以简单地运行make就可以编译整个代码树，是因为make完全按照Makefile里的指示工作。

Makefile在代码树中有很多，分别负责不同模块的编译工作。我们要修改的是kernel/Makefile。需要修改两处。一处是：

```
OBJS  = sched.o system_call.o traps.o asm.o fork.o \
        panic.o printk.o vsprintf.o sys.o exit.o \
        signal.o mktime.o
```

改为：

```
OBJS  = sched.o system_call.o traps.o asm.o fork.o \
        panic.o printk.o vsprintf.o sys.o exit.o \
        signal.o mktime.o who.o
```

另一处：

```
### Dependencies:
exit.s exit.o: exit.c ../include/errno.h ../include/signal.h \
  ../include/sys/types.h ../include/sys/wait.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
  ../include/linux/kernel.h ../include/linux/tty.h ../include/termios.h \
  ../include/asm/segment.h
```

改为：

```
### Dependencies:
who.s who.o: who.c ../include/linux/kernel.h ../include/unistd.h
exit.s exit.o: exit.c ../include/errno.h ../include/signal.h \
  ../include/sys/types.h ../include/sys/wait.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
  ../include/linux/kernel.h ../include/linux/tty.h ../include/termios.h \
  ../include/asm/segment.h
```

Makefile修改后，和往常一样“make all”就能自动把who.c加入到内核中了。如果编译时提示who.c有错误，就说明修改生效了。所以，有意或无意地制造一两个错误也不完全是坏事，至少能证明Makefile是对的。

### 用printk()调试内核

oslab实验环境提供了基于C语言和汇编语言的两种调试手段。除此之外，适当地向屏幕输出一些程序运行状态的信息，也是一种很高效、便捷的调试方法，有时甚至是唯一的方法，被称为“printf法”。

要知道到，printf()是一个只能在用户模式下执行的函数，而系统调用是在内核模式中运行，所以printf()不可用，要用printk()。它和printf的接口和功能基本相同，只是代码上有一点点不同。printk()需要特别处理一下fs寄存器，它是专用于用户模式的段寄存器。看一看printk的代码（在kernel/printk.c中）就知道了：

```
int printk(const char *fmt, ...)
{
    ……
    __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $buf\n\t"
            "pushl $0\n\t"
            "call tty_write\n\t"
            "addl $8,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (i):"ax","cx","dx");
    ……
}
```

显然，printk()首先push %fs保存这个指向用户段的寄存器，在最后pop %fs将其恢复，printk的核心仍然是调用tty_write()。查看printf()可以看到，它最终也要落实到这个函数上。

### 编写测试程序

激动地运行一下由你亲手修改过的“Linux 0.11 pro++”！然后编写一个简单的应用程序进行测试。比如在sys_iam()中向终端printk()一些信息，让应用程序调用iam()，从结果可以看出系统调用是否被真的调用到了。

可以直接在Linux 0.11环境下用vi编写（别忘了经常执行“sync”以确保内存缓冲区的数据写入磁盘），也可以在Ubuntu或Windows下编完后再传到Linux 0.11下。无论如何，最终都必须在Linux 0.11下编译。编译命令是：

```
# gcc -o iam iam.c -Wall
```

gcc的“-Wall”参数是给出所有的编译警告信息，“-o”参数指定生成的执行文件名是iam，用下面命令运行它：

```
# ./iam
```

如果如愿输出了你的信息，就说明你添加的系统调用生效了。否则，就还要继续调试，祝你好运！

### 在用户态和核心态之间传递数据

指针参数传递的是应用程序所在地址空间的逻辑地址，在内核中如果直接访问这个地址，访问到的是内核空间中的数据，不会是用户空间的。所以这里还需要一点儿特殊工作，才能在内核中从用户空间得到数据。

要实现的两个系统调用参数中都有字符串指针，非常象open(char *filename, ……)，所以我们看一下open()系统调用是如何处理的。

```
int open(const char * filename, int flag, ...)
{  
    ……
    __asm__("int $0x80"
            :"=a" (res)
            :"0" (__NR_open),"b" (filename),"c" (flag),
            "d" (va_arg(arg,int)));
    ……
}
```

可以看出，系统调用是用eax、ebx、ecx、edx寄存器来传递参数的。其中eax传递了系统调用号，而ebx、ecx、edx是用来传递函数的参数的，其中ebx对应第一个参数，ecx对应第二个参数，依此类推。如open所传递的文件名指针是由ebx传递的，也即进入内核后，通过ebx取出文件名字符串。open的ebx指向的数据在用户空间，而当前执行的是内核空间的代码，如何在用户态和核心态之间传递数据？接下来我们继续看看open的处理：

```
system_call: //所有的系统调用都从system_call开始
    ……
    pushl %edx
    pushl %ecx        
    pushl %ebx                # push %ebx,%ecx,%edx，这是传递给系统调用的参数
    movl $0x10,%edx            # 让ds,es指向GDT，指向核心地址空间
    mov %dx,%ds
    mov %dx,%es
    movl $0x17,%edx            # 让fs指向的是LDT，指向用户地址空间
    mov %dx,%fs
    call sys_call_table(,%eax,4)    # 即call sys_open
```

由上面的代码可以看出，获取用户地址空间（用户数据段）中的数据依靠的就是段寄存器fs，下面该转到sys_open执行了，在fs/open.c文件中：

```
int sys_open(const char * filename,int flag,int mode)  //filename这些参数从哪里来？
/*是否记得上面的pushl %edx,    pushl %ecx,    pushl %ebx？
  实际上一个C语言函数调用另一个C语言函数时，编译时就是将要
  传递的参数压入栈中（第一个参数最后压，…），然后call …，
  所以汇编程序调用C函数时，需要自己编写这些参数压栈的代码…*/
{
    ……
    if ((i=open_namei(filename,flag,mode,&inode))<0) {
        ……
    }
    ……
}
```

它将参数传给了open_namei()。再沿着open_namei()继续查找，文件名先后又被传给dir_namei()、get_dir()。在get_dir()中可以看到：

```
static struct m_inode * get_dir(const char * pathname)
{
    ……
    if ((c=get_fs_byte(pathname))=='/') {
        ……
    }
    ……
}
```

处理方法就很显然了：用get_fs_byte()获得一个字节的用户空间中的数据。所以，在实现iam()时，调用get_fs_byte()即可。但如何实现whoami()呢？即如何实现从核心态拷贝数据到用心态内存空间中呢？猜一猜，是否有put_fs_byte()？有！看一看include/asm/segment.h：

```
extern inline unsigned char get_fs_byte(const char * addr)
{
    unsigned register char _v;
    __asm__ ("movb %%fs:%1,%0":"=r" (_v):"m" (*addr));
    return _v;
}

extern inline void put_fs_byte(char val,char *addr)
{
    __asm__ ("movb %0,%%fs:%1"::"r" (val),"m" (*addr));
}
```

他俩以及所有put_fs_xxx()和get_fs_xxx()都是用户空间和内核空间之间的桥梁，在后面的实验中还要经常用到。

### 运行脚本程序

Linux的一大特色是可以编写功能强大的shell脚本，提高工作效率。本实验的部分评分工作由脚本 testlab2.sh 完成。它的功能是测试iam.c和whoami.c。

首先将iam.c和whoami.c分别编译成iam和whoami，然后将 testlab2.sh 拷贝到同一目录下。用下面命令为此脚本增加执行权限：

```
chmod +x testlab2.sh
```

然后运行之：

```
./testlab2.sh
```

根据输出，可知iam.c和whoami.c的得分。

### errno

errno是一种传统的错误代码返回机制。当一个函数调用出错时，通常会返回-1给调用者。但-1只能说明出错，不能说明错是什么。为解决此问题，全局变量errno登场了。错误值被存放到errno中，于是调用者就可以通过判断errno来决定如何应对错误了。各种系统对errno的值的含义都有标准定义。Linux下用“man errno”可以看到这些定义。

# 6. 进程运行轨迹的跟踪与统计

## 实验目的

- 掌握Linux下的多进程编程技术；
- 通过对进程运行轨迹的跟踪来形象化进程的概念；
- 在进程运行轨迹跟踪的基础上进行相应的数据统计，从而能对进程调度算法进行实际的量化评价，更进一步加深对调度和调度算法的理解，获得能在实际操作系统上对调度算法进行实验数据对比的直接经验。

## 实验内容

进程从创建（Linux下调用fork()）到结束的整个过程就是进程的生命期，进程在其生命期中的运行轨迹实际上就表现为进程状态的多次切换，如进程创建以后会成为就绪态；当该进程被调度以后会切换到运行态；在运行的过程中如果启动了一个文件读写操作，操作系统会将该进程切换到阻塞态（等待态）从而让出CPU；当文件读写完毕以后，操作系统会在将其切换成就绪态，等待进程调度算法来调度该进程执行……

本次实验包括如下内容：

1. 基于模板“process.c”编写多进程的样本程序，实现如下功能：
   1. 所有子进程都并行运行，每个子进程的实际运行时间一般不超过30秒；
   2. 父进程向标准输出打印所有子进程的id，并在所有子进程都退出后才退出；
2. 在Linux 0.11上实现进程运行轨迹的跟踪。基本任务是在内核中维护一个日志文件/var/process.log，把从操作系统启动到系统关机过程中所有进程的运行轨迹都记录在这一log文件中。
3. 在修改过的0.11上运行样本程序，通过分析log文件，统计该程序建立的所有进程的等待时间、完成时间（周转时间）和运行时间，然后计算平均等待时间，平均完成时间和吞吐量。可以自己编写统计程序，也可以使用python脚本程序—— stat_log.py ——进行统计。
4. 修改0.11进程调度的时间片，然后再运行同样的样本程序，统计同样的时间数据，和原有的情况对比，体会不同时间片带来的差异。

/var/process.log文件的格式必须为：

pid X time 

其中：

- pid是进程的ID；
- X可以是N,J,R,W和E中的任意一个，分别表示进程新建(N)、进入就绪态(J)、进入运行态(R)、进入阻塞态(W)和退出(E)；
- time表示X发生的时间。这个时间不是物理时间，而是系统的滴答时间(tick)；

三个字段之间用制表符分隔。 例如：

```
12    N    1056
12    J    1057
4    W    1057
12    R    1057
13    N    1058
13    J    1059
14    N    1059
14    J    1060
15    N    1060
15    J    1061
12    W    1061
15    R    1061
15    J    1076
14    R    1076
14    E    1076
……
```

## 实验报告

完成实验后，在实验报告中回答如下问题：

1. 结合自己的体会，谈谈从程序设计者的角度看，单进程编程和多进程编程最大的区别是什么？
2. 你是如何修改时间片的？仅针对样本程序建立的进程，在修改时间片前后，log文件的统计结果（不包括Graphic）都是什么样？结合你的修改分析一下为什么会这样变化，或者为什么没变化？

## 评分标准

- process.c，50%
- 日志文件建立成功，5%
- 能向日志文件输出信息，5%
- 5种状态都能输出，10%（每种2%）
- 调度算法修改，10%
- 实验报告，20%

## 实验提示

process.c的编写涉及到fork()和wait()系统调用，请自行查阅相关文献。0.11内核修改涉及到init/main.c、kernel/fork.c和kernel/sched.c，开始实验前如果能详细阅读《注释》一书的相关部分，会大有裨益。

### 编写样本程序

所谓样本程序，就是一个生成各种进程的程序。我们的对0.11的修改把系统对它们的调度情况都记录到log文件中。在修改调度算法或调度参数后再运行完全一样的样本程序，可以检验调度算法的优劣。理论上，此程序可以在任何Unix/Linux上运行，所以建议在Ubuntu上调试通过后，再拷贝到0.11下运行。

process.c是样本程序的模板。它主要实现了一个函数：

```
/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O，直到总运行时间超过last为止
 * 所有时间的单位为秒
 */
cpuio_bound(int last, int cpu_time, int io_time);
比如一个进程如果要占用10秒的CPU时间，它可以调用：

cpuio_bound(10, 1, 0);  // 只要cpu_time>0，io_time=0，效果相同
以I/O为主要任务：

cpuio_bound(10, 0, 1);  // 只要cpu_time=0，io_time>0，效果相同
CPU和I/O各1秒钟轮回：

cpuio_bound(10, 1, 1);
较多的I/O，较少的CPU：

cpuio_bound(10, 1, 9);  // I/O时间是CPU时间的9倍
```

修改此模板，用fork()建立若干个同时运行的子进程，父进程等待所有子进程退出后才退出，每个子进程按照你的意愿做不同或相同的cpuio_bound()，从而完成一个个性化的样本程序。它可以用来检验有关log文件的修改是否正确，同时还是数据统计工作的基础。

wait()系统调用可以让父进程等待子进程的退出。

#### 小技巧：

1. 在Ubuntu下，top命令可以监视即时的进程状态。在top中，按u，再输入你的用户名，可以限定只显示以你的身份运行的进程，更方便观察。按h可得到帮助。
2. 在Ubuntu下，ps命令可以显示当时各个进程的状态。“ps aux”会显示所有进程；“ps aux | grep xxxx”将只显示名为xxxx的进程。更详细的用法请问man。
3. 在Linux 0.11下，按F1可以即时显示当前所有进程的状态。

### log文件

操作系统启动后先要打开/var/process.log，然后在每个进程发生状态切换的时候向log文件内写入一条记录，其过程和用户态的应用程序没什么两样。然而，因为内核状态的存在，使过程中的很多细节变得完全不一样。

#### 打开log文件

为了能尽早开始记录，应当在内核启动时就打开log文件。内核的入口是init/main.c中的main()（Windows环境下是start()），其中一段代码是：

```
……
move_to_user_mode();
if (!fork()) {        /* we count on this going ok */
    init();
}
……
```

这段代码在进程0中运行，先切换到用户模式，然后全系统第一次调用fork()建立进程1。进程1调用init()。在init()中：

```
……
setup((void *) &drive_info);        //加载文件系统
(void) open("/dev/tty0",O_RDWR,0);    //打开/dev/tty0，建立文件描述符0和/dev/tty0的关联
(void) dup(0);                //让文件描述符1也和/dev/tty0关联
(void) dup(0);                //让文件描述符2也和/dev/tty0关联
……
```

这段代码建立了文件描述符0、1和2，它们分别就是stdin、stdout和stderr。这三者的值是系统标准（Windows也是如此），不可改变。可以把log文件的描述符关联到3。文件系统初始化，描述符0、1和2关联之后，才能打开log文件，开始记录进程的运行轨迹。为了能尽早访问log文件，我们要让上述工作在进程0中就完成。所以把这一段代码从init()移动到main()中，放在move_to_user_mode()之后（不能再靠前了），同时加上打开log文件的代码。修改后的main()如下：

```
……
move_to_user_mode();

/***************添加开始***************/
setup((void *) &drive_info);
(void) open("/dev/tty0",O_RDWR,0);    //建立文件描述符0和/dev/tty0的关联
(void) dup(0);        //文件描述符1也和/dev/tty0关联
(void) dup(0);        //文件描述符2也和/dev/tty0关联
(void) open("/var/process.log",O_CREAT|O_TRUNC|O_WRONLY,0666);
/***************添加结束***************/

if (!fork()) {        /* we count on this going ok */
    init();
}
……
```

打开log文件的参数的含义是建立只写文件，如果文件已存在则清空已有内容。文件的权限是所有人可读可写。

这样，文件描述符0、1、2和3就在进程0中建立了。根据fork()的原理，进程1会继承这些文件描述符，所以init()中就不必再open()它们。此后所有新建的进程都是进程1的子孙，也会继承它们。但实际上，init()的后续代码和/bin/sh都会重新初始化它们。所以只有进程0和进程1的文件描述符肯定关联着log文件，这一点在接下来的写log中很重要。

### 写log文件

log文件将被用来记录进程的状态转移轨迹。所有的状态转移都是在内核进行的。在内核状态下，write()功能失效，其原理等同于《系统调用》实验中不能在内核状态调用printf()，只能调用printk()。编写可在内核调用的write()的难度较大，所以这里直接给出源码。它主要参考了printk()和sys_write()而写成的：

```
#include <linux/sched.h>
#include <sys/stat.h>

static char logbuf[1024];
int fprintk(int fd, const char *fmt, ...)
{
    va_list args;
    int count;
    struct file * file;
    struct m_inode * inode;

    va_start(args, fmt);
    count=vsprintf(logbuf, fmt, args);
    va_end(args);

    if (fd < 3)    /* 如果输出到stdout或stderr，直接调用sys_write即可 */
    {
        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t" /* 注意对于Windows环境来说，是_logbuf,下同 */
            "pushl %1\n\t"
            "call sys_write\n\t" /* 注意对于Windows环境来说，是_sys_write,下同 */
            "addl $8,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (fd):"ax","cx","dx");
    }
    else    /* 假定>=3的描述符都与文件关联。事实上，还存在很多其它情况，这里并没有考虑。*/
    {
        if (!(file=task[0]->filp[fd]))    /* 从进程0的文件描述符表中得到文件句柄 */
            return 0;
        inode=file->f_inode;

        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t"
            "pushl %1\n\t"
            "pushl %2\n\t"
            "call file_write\n\t"
            "addl $12,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (file),"r" (inode):"ax","cx","dx");
    }
    return count;
}
```

因为和printk的功能近似，建议将此函数放入到kernel/printk.c中。fprintk()的使用方式类同与C标准库函数fprintf()，唯一的区别是第一个参数是文件描述符，而不是文件指针。例如：

```
fprintk(1, "The ID of running process is %ld", current->pid); //向stdout打印正在运行的进程的ID
fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'R', jiffies); //向log文件输出
```

### 跟踪进程运行轨迹

### jiffies，滴答

jiffies在kernel/sched.c文件中定义为一个全局变量：

```
long volatile jiffies=0;
```

它记录了从开机到当前时间的时钟中断发生次数。在kernel/sched.c文件中的sched_init()函数中，时钟中断处理函数被设置为：

```
set_intr_gate(0x20,&timer_interrupt);
```

而在kernel/system_call.s文件中将timer_interrupt定义为：

```
timer_interrupt:
    ……
    incl jiffies     #增加jiffies计数值
    ……
```

这说明jiffies表示从开机时到现在发生的时钟中断次数，这个数也被称为“滴答数”。

另外，在kernel/sched.c中的sched_init()中有下面的代码：

```
outb_p(0x36, 0x43); //设置8253模式
outb_p(LATCH&0xff, 0x40);
outb_p(LATCH>>8, 0x40);
```

这三条语句用来设置每次时钟中断的间隔，即为LATCH，而LATCH是定义在文件kernel/sched.c中的一个宏：

```
#define LATCH  (1193180/HZ)
#define HZ 100  //在include/linux/sched.h中
```

再加上PC机8253定时芯片的输入时钟频率为1.193180MHz，即1193180/每秒，LATCH=1193180/100，时钟每跳11931.8下产生一次时钟中断，即每1/100秒（10ms）产生一次时钟中断，所以jiffies实际上记录了从开机以来共经过了多少个10ms。

### 寻找状态切换点

必须找到所有发生进程状态切换的代码点，并在这些点添加适当的代码，来输出进程状态变化的情况到log文件中。此处要面对的情况比较复杂，需要对kernel下的fork.c、sched.c有通盘的了解，而exit.c也会涉及到。我们给出两个例子描述这个工作该如何做，其他情况实验者可仿照完成。

第一个例子是看看如何记录一个进程生命期的开始，当然这个事件就是进程的创建函数fork()，由《系统调用》实验可知，fork()功能在内核中实现为sys_fork()，该“函数”在文件kernel/system_call.s中实现为：

```
sys_fork:
    call find_empty_process
    ……
    push %gs   //传递一些参数
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %eax
    call copy_process  //调用copy_process实现进程创建
    addl $20,%esp
```

所以真正实现进程创建的函数是copy_process()，它在kernel/fork.c中定义为：

```
int copy_process(int nr,……)
{
    struct task_struct *p;
    ……
    p = (struct task_struct *) get_free_page();  //获得一个task_struct结构体空间
    ……
    p->pid = last_pid;
    ……
    p->start_time = jiffies;    //设置start_time为jiffies
       ……
    p->state = TASK_RUNNING;    //设置进程状态为就绪。所有就绪进程的状态都是
                    //TASK_RUNNING(0），被全局变量current指向的
                    //是正在运行的进程。
    return last_pid;
}
```

因此要完成进程运行轨迹的记录就要在copy_process()中添加输出语句。这里要输出两种状态，分别是“N（新建）”和“J（就绪）”。

第二个例子是记录进入睡眠态的时间。sleep_on()和interruptible_sleep_on()让当前进程进入睡眠状态，这两个函数在kernel/sched.c文件中定义如下：

```
void sleep_on(struct task_struct **p)
{
    struct task_struct *tmp;
    ……
    tmp = *p;
    *p = current;  //仔细阅读，实际上是将current插入“等待队列”头部，tmp是原来的头部
    current->state = TASK_UNINTERRUPTIBLE; //切换到睡眠态
    schedule();  //让出CPU
    if (tmp)
        tmp->state=0;    //唤醒队列中的上一个（tmp）睡眠进程。0换作TASK_RUNNING更好
                //在记录进程被唤醒时一定要考虑到这种情况，实验者一定要注意!!!
}
/* TASK_UNINTERRUPTIBLE和TASK_INTERRUPTIBLE的区别在于不可中断的睡眠
 * 只能由wake_up()显式唤醒，再由上面的 schedule()语句后的
 *
 *   if (tmp) tmp->state=0;
 *
 * 依次唤醒，所以不可中断的睡眠进程一定是按严格从“队列”（一个依靠
 * 放在进程内核栈中的指针变量tmp维护的队列）的首部进行唤醒。而对于可
 * 中断的进程，除了用wake_up唤醒以外，也可以用信号（给进程发送一个信
 * 号，实际上就是将进程PCB中维护的一个向量的某一位置位，进程需要在合
 * 适的时候处理这一位。感兴趣的实验者可以阅读有关代码）来唤醒，如在
 * schedule()中：
 *
 *  for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
 *      if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) &&
 *         (*p)->state==TASK_INTERRUPTIBLE)
 *         (*p)->state=TASK_RUNNING;//唤醒
 *
 * 就是当进程是可中断睡眠时，如果遇到一些信号就将其唤醒。这样的唤醒会
 * 出现一个问题，那就是可能会唤醒等待队列中间的某个进程，此时这个链就
 * 需要进行适当调整。interruptible_sleep_on和sleep_on函数的主要区别就
 * 在这里。
 */
void interruptible_sleep_on(struct task_struct **p)
{
    struct task_struct *tmp;
       …
    tmp=*p;
    *p=current;
repeat:    current->state = TASK_INTERRUPTIBLE;
    schedule();
    if (*p && *p != current) { //如果队列头进程和刚唤醒的进程current不是一个，说明从队列中间唤醒了一个进程，需要处理
        (**p).state=0;   //将队列头唤醒，并通过goto repeat让自己再去睡眠
        goto repeat;
    }
    *p=NULL;
    if (tmp)
        tmp->state=0;  //作用和sleep_on函数中的一样
}
```

相信实验者已经找到合适的地方插入记录进程从运行到睡眠的语句了。

总的来说，Linux 0.11支持四种进程状态的转移：就绪到运行、运行到就绪、运行到睡眠和睡眠到就绪，此外还有新建和退出两种情况。其中就绪与运行间的状态转移是通过schedule()（它亦是调度算法所在）完成的；运行到睡眠依靠的是sleep_on()和interruptible_sleep_on()，还有进程主动睡觉的系统调用sys_pause()和sys_waitpid()；睡眠到就绪的转移依靠的是wake_up()。所以只要在这些函数的适当位置插入适当的处理语句就能完成进程运行轨迹的全面跟踪了。

为了让生成的log文件更精准，以下几点请注意：

1. 进程退出的最后一步是通知父进程自己的退出，目的是唤醒正在等待此事件的父进程。从时序上来说，应该是子进程先退出，父进程才醒来。 2 .schedule()找到的next进程是接下来要运行的进程（注意，一定要分析清楚next是什么）。如果next恰好是当前正处于运行态的进程，swith_to(next)也会被调用。这种情况下相当于当前进程的状态没变。
2. 系统无事可做的时候，进程0会不停地调用sys_pause()，以激活调度算法。此时它的状态可以是等待态，等待有其它可运行的进程；也可以叫运行态，因为它是唯一一个在CPU上运行的进程，只不过运行的效果是等待。

### 管理log文件

日志文件的管理与代码编写无关，有几个要点要注意：

1. 每次关闭bochs前都要执行一下“sync”命令，它会刷新cache，确保文件确实写入了磁盘。
2. 在0.11下，可以用“ls -l /var”或“ll /var”查看process.log是否建立，及它的属性和长度。
3. 一定要实践《实验环境的搭建与使用》一章中关于文件交换的部分。最终肯定要把process.log文件拷贝到主机环境下处理。
4. 在0.11下，可以用“vi /var/process.log”或“more /var/process.log”查看整个log文件。不过，还是拷贝到Ubuntu下看，会更舒服。
5. 在0.11下，可以用“tail -n NUM /var/process.log”查看log文件的最后NUM行。

一种可能的情况下，得到的process.log文件的前几行是：

```
1    N    48    //进程1新建（init()）。此前是进程0建立和运行，但为什么没出现在log文件里？
1    J    49    //新建后进入就绪队列
0    J    49    //进程0从运行->就绪，让出CPU
1    R    49    //进程1运行
2    N    49    //进程1建立进程2。2会运行/etc/rc脚本，然后退出
2    J    49
1    W    49    //进程1开始等待（等待进程2退出）
2    R    49    //进程2运行
3    N    64    //进程2建立进程3。3是/bin/sh建立的运行脚本的子进程
3    J    64
2    E    68    //进程2不等进程3退出，就先走一步了
1    J    68    //进程1此前在等待进程2退出，被阻塞。进程2退出后，重新进入就绪队列
1    R    68
4    N    69    //进程1建立进程4，即shell
4    J    69
1    W    69    //进程1等待shell退出（除非执行exit命令，否则shell不会退出）
3    R    69    //进程3开始运行
3    W    75
4    R    75
5    N    107    //进程5是shell建立的不知道做什么的进程
5    J    108
4    W    108
5    R    108
4    J    110
5    E    111    //进程5很快退出
4    R    111
4    W    116    //shell等待用户输入命令。
0    R    116    //因为无事可做，所以进程0重出江湖
4    J    239    //用户输入命令了，唤醒了shell
4    R    239
4    W    240
0    R    240
……
```

### 数据统计

为展示实验结果，需要编写一个数据统计程序，它从log文件读入原始数据，然后计算平均周转时间、平均等待时间和吞吐率。任何语言都可以编写这样的程序，实验者可自行设计。我们用python语言编写了一个——[stat_log.py](https://github.com/traitorousfc/HIT-OSLab/blob/master/Resources/stat_log.py)（这是python源程序，可以用任意文本编辑器打开）。

python是一种跨平台的脚本语言，号称“可执行的伪代码”，非常强大，非常好用，也非常有用，建议闲着的时候学习一下。其解释器免费且开源，Ubuntu下这样安装：

```
sudo apt-get install python
```

然后只要给stat_log.py加上执行权限（chmod +x stat_log.py）就可以直接运行它。

Windows用户需要先到[http://www.python.org](http://www.python.org/)下载、安装python解释器，才能运行stat_log.py。

此程序必须在命令行下加参数执行，直接运行会打印使用说明。

```
Usage:
  ./stat_log.py /path/to/process.log [PID1] [PID2] ... [-x PID1 [PID2] ... ] [-m] [-g]
Example:
  # Include process 6, 7, 8 and 9 in statistics only. (Unit: tick)
  ./stat_log.py /path/to/process.log 6 7 8 9
  # Exclude process 0 and 1 from statistics. (Unit: tick)
  ./stat_log.py /path/to/process.log -x 0 1
  # Include process 6 and 7 only. (Unit: millisecond)
  ./stat_log.py /path/to/process.log 6 7 -m
  # Include all processes and print a COOL "graphic"! (Unit: tick)
  ./stat_log.py /path/to/process.log -g
```

运行“./stat_log.py process.log 0 1 2 3 4 5 -g”（只统计PID为0、1、2、3、4和5的进程）的输出示例：

```
(Unit: tick)
Process   Turnaround   Waiting   CPU Burst   I/O Burst
     0           75        67           8           0
     1         2518         0           1        2517
     2           25         4          21           0
     3         3003         0           4        2999
     4         5317         6          51        5260
     5            3         0           3           0
Average:     1823.50     12.83
Throughout: 0.11/s
-----===< COOL GRAPHIC OF SCHEDULER >===-----

            [Symbol]   [Meaning]
         ~~~~~~~~~~~~~~~~~~~~~~~~~~~
             number   PID or tick
              "-"     New or Exit 
              "#"       Running
              "|"        Ready
              ":"       Waiting
                    / Running with 
              "+" -|     Ready 
                    \and/or Waiting

-----===< !!!!!!!!!!!!!!!!!!!!!!!!! >===-----

  40 -0                      
  41 #0                      
  42 #                       
  43 #                       
  44 #                       
  45 #                       
  46 #                       
  47 #                       
  48 |0  -1                  
  49 |   :1  -2              
  50 |   :   #2              
  51 |   :   #               
  52 |   :   #               
  53 |   :   #               
  54 |   :   #               
  55 |   :   #               
  56 |   :   #               
  57 |   :   #               
  58 |   :   #               
  59 |   :   #               
  60 |   :   #               
  61 |   :   #               
  62 |   :   #               
  63 |   :   #               
  64 |   :   |2  -3          
  65 |   :   |   #3          
  66 |   :   |   #           
  67 |   :   |   #           
…………
```

小技巧：如果命令行程序输出过多，可以用“command arguments | more”的方式运行，结果会一屏一屏地显示。“more”在Linux和Windows下都有。Linux下还有一个“less”，和“more”类似，但功能更强，可以上下翻页、搜索。

### 修改时间片

下面是0.11的调度函数schedule，在文件kernel/sched.c中定义为：

```
while (1) {
    c = -1; next = 0; i = NR_TASKS; p = &task[NR_TASKS];
    while (--i) {
        if (!*--p)    continue;
        if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
            c = (*p)->counter, next = i;
    }                //找到counter值最大的就绪态进程
    if (c) break;    //如果有counter值大于0的就绪态进程，则退出
    for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
          if (*p) (*p)->counter = ((*p)->counter >> 1) + (*p)->priority;  
                        //如果没有，所有进程的counter值除以2衰减后再和priority值相加，产生新的时间片
}
switch_to(next);  //切换到next进程
```

分析代码可知，0.11的调度算法是选取counter值最大的就绪进程进行调度。其中运行态进程（即current）的counter数值会随着时钟中断而不断减1（时钟中断10ms一次），所以是一种比较典型的时间片轮转调度算法。另外，由上面的程序可以看出，当没有counter值大于0的就绪进程时，要对所有的进程做“(*p)->counter = ((*p)->counter >> 1) + (*p)->priority”。其效果是对所有的进程（包括阻塞态进程）都进行counter的衰减，并再累加priority值。这样，对正被阻塞的进程来说，一个进程在阻塞队列中停留的时间越长，其优先级越大，被分配的时间片也就会越大。所以总的来说，Linux 0.11的进程调度是一种综合考虑进程优先级并能动态反馈调整时间片的轮转调度算法。

此处要求实验者对现有的调度算法进行时间片大小的修改，并进行实验验证。

为完成此工作，我们需要知道两件事情：

1. 进程counter是如何初始化的？
2. 当进程的时间片用完时，被重新赋成何值？

首先回答第一个问题，显然这个值是在fork()中设定的。Linux 0.11的fork()会调用copy_process()来完成从父进程信息拷贝（所以才称其为fork），看看copy_process()的实现（也在kernel/fork.c文件中），会发现其中有下面两条语句：

```
*p = *current;            //用来复制父进程的PCB数据信息，包括priority和counter
p->counter = p->priority;    //初始化counter
```

因为父进程的counter数值已发生变化，而priority不会，所以上面的第二句代码将p->counter设置成p->priority。每个进程的priority都是继承自父亲进程的，除非它自己改变优先级。查找所有的代码，只有一个地方修改过priority，那就是nice系统调用：

```
int sys_nice(long increment)
{
    if (current->priority-increment>0)
        current->priority -= increment; 
    return 0;
}
```

本实验假定没有人调用过nice系统调用，时间片的初值就是进程0的priority，即宏INIT_TASK中定义的：

```
#define INIT_TASK \
    { 0,15,15, //分别对应state;counter;和priority;
```

接下来回答第二个问题，当就绪进程的counter为0时，不会被调度（schedule要选取counter最大的，大于0的进程），而当所有的就绪态进程的counter都变成0时，会执行下面的语句：

```
(*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
```

显然算出的新的counter值也等于priority，即初始时间片的大小。

提示就到这里。如何修改时间片，自己思考、尝试吧!



# 7. 基于内核栈切换的进程切换

## 课程说明

- L10 用户级线程
- L11 内核级线程
- L12 核心级线程实现实例
- L13 操作系统的那棵树

 ## 实验目的

- 深入理解进程和进程切换的概念；
- 综合应用进程、CPU 管理、PCB、LDT、内核栈、内核态等知识解决实际问题；
- 开始建立系统认识。

## 实验内容

现在的 Linux 0.11 采用 TSS（后面会有详细论述）和一条指令就能完成任务切换，虽然简单，但这指令的执行时间却很长，在实现任务切换时大概需要 200 多个时钟周期。

而通过堆栈实现任务切换可能要更快，而且采用堆栈的切换还可以使用指令流水的并行优化技术，同时又使得 CPU 的设计变得简单。所以无论是 Linux 还是 Windows，进程/线程的切换都没有使用 Intel 提供的这种 TSS 切换手段，而都是通过堆栈实现的。

本次实践项目就是将 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于堆栈的切换程序。具体的说，就是将 Linux 0.11 中的 `switch_to` 实现去掉，写成一段基于堆栈切换的代码。

本次实验包括如下内容：

- 编写汇编程序 `switch_to`：
- 完成主体框架；
- 在主体框架下依次完成 PCB 切换、内核栈切换、LDT 切换等；
- 修改 `fork()`，由于是基于内核栈的切换，所以进程需要创建出能完成内核栈切换的样子。
- 修改 PCB，即 `task_struct` 结构，增加相应的内容域，同时处理由于修改了 task_struct 所造成的影响。
- 用修改后的 Linux 0.11 仍然可以启动、可以正常使用。
- （选做）分析实验 3 的日志体会修改前后系统运行的差别。

## 实验报告

回答下面三个题：

#### 问题 1

针对下面的代码片段：

```
movl tss,%ecx
addl $4096,%ebx
movl %ebx,ESP0(%ecx)
```

回答问题：

- （1）为什么要加 4096；

  ebx指向的是下一个进程的PCB。加上4096后，即为一个进程分配4K的空间，栈顶即为内核堆栈的指针，栈底为进程的PCB。

- （2）为什么没有设置 tss 中的 ss0。

  因为进程的切换不靠tss进行，但CPU的机制造成对每个进程仍然会有TR寄存器、tss的设置等内容，所以可以让所有进程都共用tss0的空间。所以不需要设置tss0。

#### 问题 2

针对代码片段：

```c
*(--krnstack) = ebp;
*(--krnstack) = ecx;
*(--krnstack) = ebx;
*(--krnstack) = 0;
```

回答问题：

- （1）子进程第一次执行时，eax=？为什么要等于这个数？哪里的工作让 eax 等于这样一个数？

  这里eax等于0，即返回值。这样设置就可以用if(!fork())来判断是不是子进程。最后一行完成了这件事。 

- （2）这段代码中的 ebx 和 ecx 来自哪里，是什么含义，为什么要通过这些代码将其写到子进程的内核栈中？

  ebx、ecx都来自copy_process函数，存放的是父进程的ebx和ecx。通过将父进程的参数压入栈中，可以保证子进程回到用户态运行时具有和父进程相同的环境。

- （3）这段代码中的 ebp 来自哪里，是什么含义，为什么要做这样的设置？可以不设置吗？为什么？

  ebp也来自copy_process函数，存放的是父进程的用户栈指针。即在fork刚刚执行完copy_process的时候，它的用户栈是父进程的用户栈，而非自己的用户栈。当子进程进行其他操作时，造成需要的栈将要与父进程不同了，才会创建自己的用户栈。这么做的好处时当一些子进程什么都不做，系统不用分配额外的空间。当然也可以一创建子进程就为它分配一个新的栈，esp指向新的栈顶。

#### 问题 3

为什么要在切换完 LDT 之后要重新设置 fs=0x17？而且为什么重设操作要出现在切换完 LDT 之后，出现在 LDT 之前又会怎么样？

答：切换LDT时，会切换进程使用的用户栈。此时，会利用fs查找它指向的描述符表，并且取出隐藏部分：段基址与段限长放在cs。如果查完表不重新赋值，下次查表的时候就还会查这个表，不能起到切换LDT的作用。放在切完LDT是因为，switch_to之后下一个进程可能会查其他表，不在切完LDT的时候赋值就会影响其他表的查找。

## 实验提示

本次实验将 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于堆栈的切换程序。具体的说，就是将 Linux 0.11 中的 `switch_to` （在 kernal/system_call.s 中）实现去掉，写成一段基于堆栈切换的代码。

### TSS 切换

在现在的 Linux 0.11 中，真正完成进程切换是依靠任务状态段（Task State Segment，简称 TSS）的切换来完成的。

具体的说，在设计“Intel 架构”（即 x86 系统结构）时，每个任务（进程或线程）都对应一个独立的 TSS，TSS 就是内存中的一个结构体，里面包含了几乎所有的 CPU 寄存器的映像。有一个任务寄存器（Task Register，简称 TR）指向当前进程对应的 TSS 结构体，所谓的 TSS 切换就将 CPU 中几乎所有的寄存器都复制到 TR 指向的那个 TSS 结构体中保存起来，同时找到一个目标 TSS，即要切换到的下一个进程对应的 TSS，将其中存放的寄存器映像“扣在” CPU 上，就完成了执行现场的切换，如下图所示。

![图片描述信息](https://doc.shiyanlou.com/userid19614labid571time1424053399458/wm)

图 1 基于 TSS 的进程切换

Intel 架构不仅提供了 TSS 来实现任务切换，而且只要一条指令就能完成这样的切换，即图中的 ljmp 指令。

具体的工作过程是：

- （1）首先用 TR 中存取的段选择符在 GDT 表中找到当前 TSS 的内存位置，由于 TSS 是一个段，所以需要用段表中的一个描述符来表示这个段，和在系统启动时论述的内核代码段是一样的，那个段用 GDT 中的某个表项来描述，还记得是哪项吗？是 8 对应的第 1 项。此处的 TSS 也是用 GDT 中的某个表项描述，而 TR 寄存器是用来表示这个段用 GDT 表中的哪一项来描述，所以 TR 和 CS、DS 等寄存器的功能是完全类似的。
- （2）找到了当前的 TSS 段（就是一段内存区域）以后，将 CPU 中的寄存器映像存放到这段内存区域中，即拍了一个快照。
- （3）存放了当前进程的执行现场以后，接下来要找到目标进程的现场，并将其扣在 CPU 上，找目标 TSS 段的方法也是一样的，因为找段都要从一个描述符表中找，描述 TSS 的描述符放在 GDT 表中，所以找目标 TSS 段也要靠 GDT 表，当然只要给出目标 TSS 段对应的描述符在 GDT 表中存放的位置——段选择子就可以了，仔细想想系统启动时那条著名的 `jmpi 0, 8` 指令，这个段选择子就放在 ljmp 的参数中，实际上就 `jmpi 0, 8` 中的 8。
- （4）一旦将目标 TSS 中的全部寄存器映像扣在 CPU 上，就相当于切换到了目标进程的执行现场了，因为那里有目标进程停下时的 `CS:EIP`，所以此时就开始从目标进程停下时的那个 `CS:EIP` 处开始执行，现在目标进程就变成了当前进程，所以 TR 需要修改为目标 TSS 段在 GDT 表中的段描述符所在的位置，因为 TR 总是指向当前 TSS 段的段描述符所在的位置。

上面给出的这些工作都是一句长跳转指令 `ljmp 段选择子:段内偏移`，在段选择子指向的段描述符是 TSS 段时 CPU 解释执行的结果，所以基于 TSS 进行进程/线程切换的 `switch_to` 实际上就是一句 `ljmp` 指令：

```c
#define switch_to(n) {
    struct{long a,b;} tmp;
    __asm__(
        "movw %%dx,%1"
        "ljmp %0" ::"m"(*&tmp.a), "m"(*&tmp.b), "d"(TSS(n)
    )
 }

#define FIRST_TSS_ENTRY 4

#define TSS(n) (((unsigned long) n) << 4) + (FIRST_TSS_ENTRY << 3))
```

GDT 表的结构如下图所示，所以第一个 TSS 表项，即 0 号进程的 TSS 表项在第 4 个位置上，4<<3，即 `4 * 8`，相当于 TSS 在 GDT 表中开始的位置（以字节为单位），TSS（n）找到的是进程 n 的 TSS 位置，所以还要再加上 n<<4，即 `n * 16`，因为每个进程对应有 1 个 TSS 和 1 个 LDT，每个描述符的长度都是 8 个字节，所以是乘以 16，其中 LDT 的作用就是上面论述的那个映射表，关于这个表的详细论述要等到内存管理一章。`TSS(n) = n * 16 + 4 * 8`，得到就是进程 n（切换到的目标进程）的 TSS 选择子，将这个值放到 dx 寄存器中，并且又放置到结构体 tmp 中 32 位长整数 b 的前 16 位，现在 64 位 tmp 中的内容是前 32 位为空，这个 32 位数字是段内偏移，就是 `jmpi 0, 8` 中的 0；接下来的 16 位是 `n * 16 + 4 * 8`，这个数字是段选择子，就是 `jmpi 0, 8` 中的 8，再接下来的 16 位也为空。所以 swith_to 的核心实际上就是 `ljmp 空, n*16+4*8`，现在和前面给出的基于 TSS 的进程切换联系在一起了。

![图片描述信息](https://doc.shiyanlou.com/userid19614labid571time1424053507670/wm)

图 2 GDT 表中的内容

### 本次实验的内容

虽然用一条指令就能完成任务切换，但这指令的执行时间却很长，这条 ljmp 指令在实现任务切换时大概需要 200 多个时钟周期。而通过堆栈实现任务切换可能要更快，而且采用堆栈的切换还可以使用指令流水的并行优化技术，同时又使得 CPU 的设计变得简单。所以无论是 Linux 还是 Windows，进程/线程的切换都没有使用 Intel 提供的这种 TSS 切换手段，而都是通过堆栈实现的。

本次实践项目就是将 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于堆栈的切换程序。具体的说，就是将 Linux 0.11 中的 switch_to 实现去掉，写成一段基于堆栈切换的代码。

在现在的 Linux 0.11 中，真正完成进程切换是依靠任务状态段（Task State Segment，简称 TSS）的切换来完成的。具体的说，在设计“Intel 架构”（即 x86 系统结构）时，每个任务（进程或线程）都对应一个独立的 TSS，TSS 就是内存中的一个结构体，里面包含了几乎所有的 CPU 寄存器的映像。有一个任务寄存器（Task Register，简称 TR）指向当前进程对应的 TSS 结构体，所谓的 TSS 切换就将 CPU 中几乎所有的寄存器都复制到 TR 指向的那个 TSS 结构体中保存起来，同时找到一个目标 TSS，即要切换到的下一个进程对应的 TSS，将其中存放的寄存器映像“扣在”CPU 上，就完成了执行现场的切换。

要实现基于内核栈的任务切换，主要完成如下三件工作：

- （1）重写 `switch_to`；
- （2）将重写的 `switch_to` 和 `schedule()` 函数接在一起；
- （3）修改现在的 `fork()`。

### schedule 与 switch_to

目前 Linux 0.11 中工作的 schedule() 函数是首先找到下一个进程的数组位置 next，而这个 next 就是 GDT 中的 n，所以这个 next 是用来找到切换后目标 TSS 段的段描述符的，一旦获得了这个 next 值，直接调用上面剖析的那个宏展开 switch_to(next);就能完成如图 TSS 切换所示的切换了。

现在，我们不用 TSS 进行切换，而是采用切换内核栈的方式来完成进程切换，所以在新的 switch_to 中将用到当前进程的 PCB、目标进程的 PCB、当前进程的内核栈、目标进程的内核栈等信息。由于 Linux 0.11 进程的内核栈和该进程的 PCB 在同一页内存上（一块 4KB 大小的内存），其中 PCB 位于这页内存的低地址，栈位于这页内存的高地址；另外，由于当前进程的 PCB 是用一个全局变量 current 指向的，所以只要告诉新 switch_to()函数一个指向目标进程 PCB 的指针就可以了。同时还要将 next 也传递进去，虽然 TSS(next)不再需要了，但是 LDT(next)仍然是需要的，也就是说，现在每个进程不用有自己的 TSS 了，因为已经不采用 TSS 进程切换了，但是每个进程需要有自己的 LDT，地址分离地址还是必须要有的，而进程切换必然要涉及到 LDT 的切换。

综上所述，需要将目前的 `schedule()` 函数（在 `kernal/sched.c` 中）做稍许修改，即将下面的代码：

```c
if ((*p)->state == TASK_RUNNING && (*p)->counter > c) 
    c = (*p)->counter, next = i; 

//......

switch_to(next);
```

修改为：

```c
if ((*p)->state == TASK_RUNNING && (*p)->counter > c) 
    c = (*p)->counter, next = i, pnext = *p;

//.......

switch_to(pnext, LDT(next)); 
```

### 实现 switch_to

实现 `switch_to` 是本次实践项目中最重要的一部分。

由于要对内核栈进行精细的操作，所以需要用汇编代码来完成函数 `switch_to` 的编写。

这个函数依次主要完成如下功能：由于是 C 语言调用汇编，所以需要首先在汇编中处理栈帧，即处理 `ebp` 寄存器；接下来要取出表示下一个进程 PCB 的参数，并和 `current` 做一个比较，如果等于 current，则什么也不用做；如果不等于 current，就开始进程切换，依次完成 PCB 的切换、TSS 中的内核栈指针的重写、内核栈的切换、LDT 的切换以及 PC 指针（即 CS:EIP）的切换。

```
switch_to:
    pushl %ebp
    movl %esp,%ebp
    pushl %ecx
    pushl %ebx
    pushl %eax
    movl 8(%ebp),%ebx
    cmpl %ebx,current
    je 1f
! 切换PCB
    ! ...
! TSS中的内核栈指针的重写
    ! ...
! 切换内核栈
    ! ...
! 切换LDT
    ! ...
    movl $0x17,%ecx
    mov %cx,%fs
! 和后面的 clts 配合来处理协处理器，由于和主题关系不大，此处不做论述
    cmpl %eax,last_task_used_math 
    jne 1f
    clts

1:    popl %eax
    popl %ebx
    popl %ecx
    popl %ebp
ret
```

虽然看起来完成了挺多的切换，但实际上每个部分都只有很简单的几条指令。完成 PCB 的切换可以采用下面两条指令，其中 ebx 是从参数中取出来的下一个进程的 PCB 指针，

```
movl %ebx,%eax
xchgl %eax,current
```

经过这两条指令以后，eax 指向现在的当前进程，ebx 指向下一个进程，全局变量 current 也指向下一个进程。

TSS 中的内核栈指针的重写可以用下面三条指令完成，其中宏 `ESP0 = 4`，`struct tss_struct *tss = &(init_task.task.tss);` 也是定义了一个全局变量，和 current 类似，用来指向那一段 0 号进程的 TSS 内存。

前面已经详细论述过，在中断的时候，要找到内核栈位置，并将用户态下的 `SS:ESP`，`CS:EIP` 以及 `EFLAGS` 这五个寄存器压到内核栈中，这是沟通用户栈（用户态）和内核栈（内核态）的关键桥梁，而找到内核栈位置就依靠 TR 指向的当前 TSS。

现在虽然不使用 TSS 进行任务切换了，但是 Intel 的这态中断处理机制还要保持，所以仍然需要有一个当前 TSS，这个 TSS 就是我们定义的那个全局变量 tss，即 0 号进程的 tss，所有进程都共用这个 tss，任务切换时不再发生变化。

```
movl tss,%ecx
addl $4096,%ebx
movl %ebx,ESP0(%ecx)
```

定义 `ESP0 = 4` 是因为 TSS 中内核栈指针 esp0 就放在偏移为 4 的地方，看一看 tss 的结构体定义就明白了。

完成内核栈的切换也非常简单，和我们前面给出的论述完全一致，将寄存器 esp（内核栈使用到当前情况时的栈顶位置）的值保存到当前 PCB 中，再从下一个 PCB 中的对应位置上取出保存的内核栈栈顶放入 esp 寄存器，这样处理完以后，再使用内核栈时使用的就是下一个进程的内核栈了。

由于现在的 Linux 0.11 的 PCB 定义中没有保存内核栈指针这个域（kernelstack），所以需要加上，而宏 `KERNEL_STACK` 就是你加的那个位置，当然将 kernelstack 域加在 task_struct 中的哪个位置都可以，但是在某些汇编文件中（主要是在 `kernal/system_call.s` 中）有些关于操作这个结构一些汇编硬编码，所以一旦增加了 kernelstack，这些硬编码需要跟着修改，由于第一个位置，即 long state 出现的汇编硬编码很多，所以 kernelstack 千万不要放置在 task_struct 中的第一个位置，当放在其他位置时，修改 `kernal/system_call.s` 中的那些硬编码就可以了。

```
KERNEL_STACK = 12
movl %esp,KERNEL_STACK(%eax)
! 再取一下 ebx，因为前面修改过 ebx 的值
movl 8(%ebp),%ebx
movl KERNEL_STACK(%ebx),%esp
```

task_struct 的定义：

```c
// 在 include/linux/sched.h 中
struct task_struct {
    long state;
    long counter;
    long priority;
    long kernelstack;
//......
```

由于这里将 PCB 结构体的定义改变了，所以在产生 0 号进程的 PCB 初始化时也要跟着一起变化，需要将原来的 `#define INIT_TASK { 0,15,15, 0,{{},},0,...` 修改为 `#define INIT_TASK { 0,15,15,PAGE_SIZE+(long)&init_task, 0,{{},},0,...`，即在 PCB 的第四项中增加关于内核栈栈指针的初始化。

再下一个切换就是 LDT 的切换了，指令 `movl 12(%ebp),%ecx` 负责取出对应 LDT(next)的那个参数，指令 `lldt %cx` 负责修改 LDTR 寄存器，一旦完成了修改，下一个进程在执行用户态程序时使用的映射表就是自己的 LDT 表了，地址空间实现了分离。

最后一个切换是关于 PC 的切换，和前面论述的一致，依靠的就是 `switch_to` 的最后一句指令 ret，虽然简单，但背后发生的事却很多：`schedule()` 函数的最后调用了这个 `switch_to` 函数，所以这句指令 ret 就返回到下一个进程（目标进程）的 `schedule()` 函数的末尾，遇到的是}，继续 ret 回到调用的 `schedule()` 地方，是在中断处理中调用的，所以回到了中断处理中，就到了中断返回的地址，再调用 iret 就到了目标进程的用户态程序去执行，和书中论述的内核态线程切换的五段论是完全一致的。

这里还有一个地方需要格外注意，那就是 switch_to 代码中在切换完 LDT 后的两句，即：

```
! 切换 LDT 之后
movl $0x17,%ecx
mov %cx,%fs
```

这两句代码的含义是重新取一下段寄存器 fs 的值，这两句话必须要加、也必须要出现在切换完 LDT 之后，这是因为在实践项目 2 中曾经看到过 fs 的作用——通过 fs 访问进程的用户态内存，LDT 切换完成就意味着切换了分配给进程的用户态内存地址空间，所以前一个 fs 指向的是上一个进程的用户态内存，而现在需要执行下一个进程的用户态内存，所以就需要用这两条指令来重取 fs。

不过，细心的读者可能会发现：fs 是一个选择子，即 fs 是一个指向描述符表项的指针，这个描述符才是指向实际的用户态内存的指针，所以上一个进程和下一个进程的 fs 实际上都是 0x17，真正找到不同的用户态内存是因为两个进程查的 LDT 表不一样，所以这样重置一下 `fs=0x17` 有用吗，有什么用？要回答这个问题就需要对段寄存器有更深刻的认识，实际上段寄存器包含两个部分：显式部分和隐式部分，如下图给出实例所示，就是那个著名的 `jmpi 0, 8`，虽然我们的指令是让 `cs=8`，但在执行这条指令时，会在段表（GDT）中找到 8 对应的那个描述符表项，取出基地址和段限长，除了完成和 eip 的累加算出 PC 以外，还会将取出的基地址和段限长放在 cs 的隐藏部分，即图中的基地址 0 和段限长 7FF。为什么要这样做？下次执行 `jmp 100` 时，由于 cs 没有改过，仍然是 8，所以可以不再去查 GDT 表，而是直接用其隐藏部分中的基地址 0 和 100 累加直接得到 PC，增加了执行指令的效率。现在想必明白了为什么重新设置 fs=0x17 了吧？而且为什么要出现在切换完 LDT 之后？

![图片描述信息](https://doc.shiyanlou.com/userid19614labid571time1424053856897/wm)

图 3 段寄存器中的两个部分



### 修改 fork

开始修改 fork() 了，和书中论述的原理一致，就是要把进程的用户栈、用户程序和其内核栈通过压在内核栈中的 `SS:ESP`，`CS:IP` 关联在一起。

另外，由于 fork() 这个叉子的含义就是要让父子进程共用同一个代码、数据和堆栈，现在虽然是使用内核栈完成任务切换，但 fork() 的基本含义不会发生变化。

将上面两段描述联立在一起，修改 fork() 的核心工作就是要形成如下图所示的子进程内核栈结构。

![图片描述信息](https://doc.shiyanlou.com/userid19614labid571time1424053880667/wm)

图 4 fork 进程的父子进程结构

不难想象，对 fork() 的修改就是对子进程的内核栈的初始化，在 fork() 的核心实现 `copy_process` 中，`p = (struct task_struct *) get_free_page();`用来完成申请一页内存作为子进程的 PCB，而 p 指针加上页面大小就是子进程的内核栈位置，所以语句 `krnstack = (long *) (PAGE_SIZE + (long) p);` 就可以找到子进程的内核栈位置，接下来就是初始化 krnstack 中的内容了。

```c
*(--krnstack) = ss & 0xffff;
*(--krnstack) = esp;
*(--krnstack) = eflags;
*(--krnstack) = cs & 0xffff;
*(--krnstack) = eip;
```

这五条语句就完成了上图所示的那个重要的关联，因为其中 ss,esp 等内容都是 `copy_proces()` 函数的参数，这些参数来自调用 `copy_proces()` 的进程的内核栈中，就是父进程的内核栈中，所以上面给出的指令不就是将父进程内核栈中的前五个内容拷贝到子进程的内核栈中，图中所示的关联不也就是一个拷贝吗？

接下来的工作就需要和 switch_to 接在一起考虑了，故事从哪里开始呢？回顾一下前面给出来的 switch_to，应该从 “切换内核栈” 完事的那个地方开始，现在到子进程的内核栈开始工作了，接下来做的四次弹栈以及 ret 处理使用的都是子进程内核栈中的东西，

```
1: popl %eax
    popl %ebx
    popl %ecx
    popl %ebp
ret
```

为了能够顺利完成这些弹栈工作，子进程的内核栈中应该有这些内容，所以需要对 krnstack 进行初始化：

```c
*(--krnstack) = ebp;
*(--krnstack) = ecx;
*(--krnstack) = ebx;
// 这里的 0 最有意思。
*(--krnstack) = 0;
```

现在到了 ret 指令了，这条指令要从内核栈中弹出一个 32 位数作为 EIP 跳去执行，所以需要弄一个函数地址（仍然是一段汇编程序，所以这个地址是这段汇编程序开始处的标号）并将其初始化到栈中。我们弄的一个名为 `first_return_from_kernel` 的汇编标号，然后可以用语句 `*(--krnstack) = (long) first_return_from_kernel;` 将这个地址初始化到子进程的内核栈中，现在执行 ret 以后就会跳转到 `first_return_from_kernel` 去执行了。

想一想 `first_return_from_kernel` 要完成什么工作？PCB 切换完成、内核栈切换完成、LDT 切换完成，接下来应该那个“内核级线程切换五段论”中的最后一段切换了，即完成用户栈和用户代码的切换，依靠的核心指令就是 iret，当然在切换之前应该回复一下执行现场，主要就是 `eax,ebx,ecx,edx,esi,edi,gs,fs,es,ds` 等寄存器的恢复.

下面给出了 `first_return_from_kernel` 的核心代码，当然 edx 等寄存器的值也应该先初始化到子进程内核栈，即 krnstack 中。

```
popl %edx
popl %edi
popl %esi
pop %gs
pop %fs
pop %es
pop %ds
iret
```

最后别忘了将存放在 PCB 中的内核栈指针修改到初始化完成时内核栈的栈顶，即：

```c
p->kernelstack = stack;
```

# 8.信号量的实现和应用

## 实验目的

- 加深对进程同步与互斥概念的认识；
- 掌握信号量的使用，并应用它解决生产者——消费者问题；
- 掌握信号量的实现原理。

## 实验内容

本次实验的基本内容是：

1. 在Ubuntu下编写程序，用信号量解决生产者——消费者问题；
2. 在0.11中实现信号量，用生产者—消费者程序检验之。
3. 用信号量解决生产者—消费者问题

在Ubuntu上编写应用程序“pc.c”，解决经典的生产者—消费者问题，完成下面的功能：

1. 建立一个生产者进程，N个消费者进程（N>1）；
2. 用文件建立一个共享缓冲区；
3. 生产者进程依次向缓冲区写入整数0,1,2,...,M，M>=500；
4. 消费者进程从缓冲区读数，每次读一个，并将读出的数字从缓冲区删除，然后将本进程ID和数字输出到标准输出；
5. 缓冲区同时最多只能保存10个数。

一种可能的输出效果是：

```
10: 0
10: 1
10: 2
10: 3
10: 4
11: 5
11: 6
12: 7
10: 8
12: 9
12: 10
12: 11
12: 12
……
11: 498
11: 499
```

其中ID的顺序会有较大变化，但冒号后的数字一定是从0开始递增加一的。

pc.c中将会用到sem_open()、sem_close()、sem_wait()和sem_post()等信号量相关的系统调用，请查阅相关文档。

《UNIX环境高级编程》是一本关于Unix/Linux系统级编程的相当经典的教程。校园网用户可以在 ftp://run.hit.edu.cn/study/Computer_Science/Linux_Unix/ 下载，后续实验也用得到。如果你对POSIX编程感兴趣，建议买一本常备手边。

### 实现信号量

Linux在0.11版还没有实现信号量，Linus把这件富有挑战的工作留给了你。如果能实现一套山寨版的完全符合POSIX规范的信号量，无疑是很有成就感的。但时间暂时不允许我们这么做，所以先弄一套缩水版的类POSIX信号量，它的函数原型和标准并不完全相同，而且只包含如下系统调用：

```
sem_t *sem_open(const char *name, unsigned int value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_unlink(const char *name);
```

sem_t是信号量类型，根据实现的需要自定义。

sem_open()的功能是创建一个信号量，或打开一个已经存在的信号量。

name是信号量的名字。不同的进程可以通过提供同样的name而共享同一个信号量。如果该信号量不存在，就创建新的名为name的信号量；如果存在，就打开已经存在的名为name的信号量。 value是信号量的初值，仅当新建信号量时，此参数才有效，其余情况下它被忽略。 当成功时，返回值是该信号量的唯一标识（比如，在内核的地址、ID等），由另两个系统调用使用。如失败，返回值是NULL。 sem_wait()就是信号量的P原子操作。如果继续运行的条件不满足，则令调用进程等待在信号量sem上。返回0表示成功，返回-1表示失败。

sem_post()就是信号量的V原子操作。如果有等待sem的进程，它会唤醒其中的一个。返回0表示成功，返回-1表示失败。

sem_unlink()的功能是删除名为name的信号量。返回0表示成功，返回-1表示失败。

在kernel目录下新建“sem.c”文件实现如上功能。然后将pc.c从Ubuntu移植到0.11下，测试自己实现的信号量。

## 实验报告

完成实验后，在实验报告中回答如下问题：

1. 在pc.c中去掉所有与信号量有关的代码，再运行程序，执行效果有变化吗？为什么会这样？

2. 实验的设计者在第一次编写生产者——消费者程序的时候，是这么做的：

   ```
    Producer()
    {
        P(Mutex);  //互斥信号量
        生产一个产品item;
        P(Empty);  //空闲缓存资源
        将item放到空闲缓存中;
        V(Full);  //产品资源
        V(Mutex);
    }
   
    Consumer()
    {
        P(Mutex);  
        P(Full);  
        从缓存区取出一个赋值给item;
        V(Empty);
        消费产品item;
        V(Mutex);
    } 
   ```

这样可行吗？如果可行，那么它和标准解法在执行效果上会有什么不同？如果不可行，那么它有什么问题使它不可行？

## 评分标准

- pc.c，40%
- sem_open()，10%
- sem_post()，10%
- sem_wait()，10%
- sem_unlink()，10%
- 实验报告，20%

## 实验提示

### 信号量

信号量，英文为semaphore，最早由荷兰科学家、图灵奖获得者E. W. Dijkstra设计，任何操作系统教科书的“进程同步”部分都会有详细叙述。

Linux的信号量秉承POSIX规范，用“man sem_overview”可以查看相关信息。本次实验涉及到的信号量系统调用包括：sem_open()、sem_wait()、sem_post()和sem_unlink()。

生产者—消费者问题

生产者—消费者问题的解法几乎在所有操作系统教科书上都有，其基本结构为：

```
Producer()
{
    生产一个产品item;
    P(Empty);  //空闲缓存资源
    P(Mutex);  //互斥信号量
    将item放到空闲缓存中;
    V(Mutex);
    V(Full);  //产品资源
}

Consumer()
{
    P(Full);  
    P(Mutex);  
    从缓存区取出一个赋值给item;
    V(Mutex);
    V(Empty);
    消费产品item;
} 
```

显然在演示这一过程时需要创建两类进程，一类执行函数Producer()，另一类执行函数Consumer()。

### 多进程共享文件

在Linux下使用C语言，可以通过三种方法进行文件的读写：

1. 使用标准C的fopen()、fread()、fwrite()、fseek()和fclose()等；
2. 使用系统调用open()、read()、write()、lseek()和close()等；
3. 通过内存镜像文件，使用mmap()系统调用。

在Linux 0.11上只能使用前两种方法。

fork()调用成功后，子进程会继承父进程拥有的大多数资源，包括父进程打开的文件。所以子进程可以直接使用父进程创建的文件指针/描述符/句柄，访问的是与父进程相同的文件。

使用标准C的文件操作函数要注意，它们使用的是进程空间内的文件缓冲区，父进程和子进程之间不共享这个缓冲区。因此，任何一个进程做完写操作后，必须fflush()一下，将数据强制更新到磁盘，其它进程才能读到所需数据。

建议直接使用系统调用进行文件操作。

### 终端也是临界资源

用printf()向终端输出信息是很自然的事情，但当多个进程同时输出时，终端也成为了一个临界资源，需要做好互斥保护，否则输出的信息可能错乱。

另外，printf()之后，信息只是保存在输出缓冲区内，还没有真正送到终端上，这也可能造成输出信息时序不一致。用fflush(stdout)可以确保数据送到终端。

### 原子操作、睡眠和唤醒

Linux 0.11是一个支持并发的现代操作系统，虽然它还没有面向应用实现任何锁或者信号量，但它内部一定使用了锁机制，即在多个进程访问共享的内核数据时一定需要通过锁来实现互斥和同步。锁必然是一种原子操作。通过模仿0.11的锁，就可以实现信号量。

多个进程对磁盘的并发访问是一个需要锁的地方。Linux 0.11访问磁盘的基本处理办法是在内存中划出一段磁盘缓存，用来加快对磁盘的访问。进程提出的磁盘访问请求首先要到磁盘缓存中去找，如果找到直接返回；如果没有找到则申请一段空闲的磁盘缓存，以这段磁盘缓存为参数发起磁盘读写请求。请求发出后，进程要睡眠等待（因为磁盘读写很慢，应该让出CPU让其他进程执行）。这种方法是许多操作系统（包括现代Linux、UNIX等）采用的较通用的方法。这里涉及到多个进程共同操作磁盘缓存，而进程在操作过程可能会被调度而失去CPU。因此操作磁盘缓存时需要考虑互斥问题，所以其中必定用到了锁。而且也一定用到了让进程睡眠和唤醒。

下面是从kernel/blk_drv/ll_rw_blk.c文件中取出的两个函数：

```
static inline void lock_buffer(struct buffer_head * bh)
{
    cli();        //关中断
    while (bh->b_lock)
        sleep_on(&bh->b_wait);    //将当前进程睡眠在bh->b_wait
    bh->b_lock=1;
    sti();        //开中断
}

static inline void unlock_buffer(struct buffer_head * bh)
{
    if (!bh->b_lock)
        printk("ll_rw_block.c: buffer not locked\n\r");
    bh->b_lock = 0;
    wake_up(&bh->b_wait);    //唤醒睡眠在bh->b_wait上的进程
}
```

分析lock_buffer()可以看出，访问锁变量时用开、关中断来实现原子操作，阻止进程切换的发生。当然这种方法有缺点，且不适合用于多处理器环境中，但对于Linux 0.11，它是一种简单、直接而有效的机制。

另外，上面的函数表明Linux 0.11提供了这样的接口：用sleep_on()实现进程的睡眠，用wake_up()实现进程的唤醒。它们的参数都是一个结构体指针——struct task_struct *，即进程都睡眠或唤醒在该参数指向的一个进程PCB结构链表上。

因此，我们可以用开关中断的方式实现原子操作，而调用sleep_on()和wake_up()进行进程的睡眠和唤醒。

sleep_on()的功能是将当前进程睡眠在参数指定的链表上（注意，这个链表是一个隐式链表，详见《注释》一书）。wake_up()的功能是唤醒链表上睡眠的所有进程。这些进程都会被调度运行，所以它们被唤醒后，还要重新判断一下是否可以继续运行。可参考lock_buffer()中的那个while循环。

### 应对混乱的bochs虚拟屏幕

不知是Linux 0.11还是bochs的bug，如果向终端输出的信息较多，bochs的虚拟屏幕会产生混乱。此时按ctrl+L可以重新初始化一下屏幕，但输出信息一多，还是会混乱。建议把输出信息重定向到一个文件，然后用vi、more等工具按屏查看这个文件，可以基本解决此问题。

### string.h

下面描述的问题未必具有普遍意义，仅做为提醒，请实验者注意。

include/string.h实现了全套的C语言字符串操作，而且都是采用汇编+inline方式优化。但在使用中，某些情况下可能会遇到一些奇怪的问题。比如某人就遇到strcmp()会破坏参数内容的问题。如果调试中遇到有些“诡异”的情况，可以试试不包含头文件，一般都能解决。不包含string.h，就不会用inline方式调用这些函数，它们工作起来就趋于正常了。

如果遇到类似问题，欢迎到论坛说明，进行更深入的分析。