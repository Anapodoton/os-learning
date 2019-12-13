<!--ts-->

   * [os-learning](#os-learning)
      * [实验任务](#实验任务)
   * [1. 操作系统基础](#1-操作系统基础)
      * [1.1 操作系统的启动](#11-操作系统的启动)
         * [1.1.1 <strong>bootsect.S</strong>](#111-bootsects)
         * [1.1.2 setup.s](#112-setups)
         * [1.1.3 head.s](#113-heads)
      * [1.2 初始化](#12-初始化)
      * [1.3 Linux系统调用](#13-linux系统调用)
   * [2. 进程与线程](#2-进程与线程)
      * [2.1 进程运行轨迹](#21-进程运行轨迹)
         * [2.1.1  fork.c](#211--forkc)
         * [2.1.2 <strong>sched.c</strong>](#212-schedc)
         * [2.1.3 exit.c](#213-exitc)
      * [2.2 基于内核栈切换的进程切换](#22-基于内核栈切换的进程切换)
         * [2.2.1 用户级线程](#221-用户级线程)

<!-- Added by: anapodoton, at: 2019年12月 9日 星期一 18时42分05秒 CST -->

<!--te-->

# os-learning

os-learning



## 实验任务

- 熟悉 hit-oslab 实验环境
- 修改 bootsect.s 和 setup.s 完成系统引导
- 添加两个系统调用并用于测试程序
- 基于模板 “process.c” 编写多进程的样本程序并输出日志
- 重写 switch_to 完成堆栈切换实现 TSSS
- 用信号量解决生产者—消费者问题
- Bochs 调试工具跟踪 Linux 0.11 的地址映射过程、为程序增加共享内存功能
- 修改 Linux 0.11 对键盘输入和字符显示进行控制
- 在 Linux 0.11 上实现 proc 文件系统内的 psinfo 结点

# 1. 操作系统基础

我们首先介绍了什么是操作系统？然后我们学习了计算机的发展。

从白纸到图灵机（只可以计算），从图灵机到通用图灵机（可以做其他工作），从通用图灵机到计算机。提出了存储计算的思想。

下面让我们来研究下操作系统是怎么启动的。

## 1.1 操作系统的启动

来自《Linux内核完全注释》第六章。

CS为代码段寄存器，IP为指令指针寄存器。

当 PC 的电源打开后 ：

**(1) x86 PC**刚开机时**CPU**处于实模式

 **(2)**开机时，**CS=0xFFFF; IP=0x0000**

(3)**寻址**0xFFFF0(ROM BIOS**映射区**)

 (4)检查**RAM**，键盘，显示器，软硬磁盘

 **(5)** 将磁盘**0**磁道**0**扇区读入**0x7c00**处 ，从磁盘引导扇区读入的那**512**个字节 ，硬盘的第一个扇区上存放着开机后执行的第一段我们可以控制的程序。  

**(6)** 设置**cs=0x07c0**，**ip=0x0000** 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205231815.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205231920.png)

### 1.1.1 **bootsect.S**  

bootsect.S 代码是磁盘引导块程序，驻留在磁盘的第一个扇区中(引导扇区，0 磁道(柱面)，0 磁头， 第 1 个扇区)。在 PC 机加电、ROM BIOS 自检后，ROM BIOS 会把引导扇区代码 bootsect 加载到内存地 址 0x7C00 开始处并执行之。在 bootsect 代码执行期间，它会将自己移动到内存绝对地址 0x90000 开始处 并继续执行。该程序的主要作用是首先把从磁盘第 2 个扇区开始的 4 个扇区的 setup 模块(由 setup.s 编 译而成)加载到内存紧接着 bootsect 后面位置处(0x90200)，然后利用 BIOS 中断 0x13 取磁盘参数表中 当前启动引导盘的参数，接着在屏幕上显示“Loading system...”字符串。再者把磁盘上 setup 模块后面的 system 模块加载到内存 0x10000 开始的地方。随后确定根文件系统的设备号。若没有指定，则根据所保 存的引导盘的每磁道扇区数判别出盘的类型和种类(是 1.44M A 盘吗?)，并保存其设备号于 root_dev (引导块的 508 地址处)中。最后长跳转到 setup 程序开始处(0x90200)去执行 setup 程序。在磁盘上， 引导块、setup 模块和 system 模块的扇区位置和大小示意图见图 6-3 所示。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205233454.png)



![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206000136.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206000248.png)

### 1.1.2 setup.s

setup.S 是一个操作系统加载程序，它的主要作用是利用 ROM BIOS 中断读取机器系统数据，并将这 些数据保存到 0x90000 开始的位置(覆盖掉了 bootsect 程序所在的地方)。 

然后 setup 程序将 system 模块从 0x10000-0x8ffff 整块向下移动到内存绝对地址 0x00000 处(当时认 为内核系统模块 system 的长度不会超过此值:512KB)。接下来加载中断描述符表寄存器(IDTR)和全局 描述符表寄存器(GDTR)，开启 A20 地址线，重新设置两个中断控制芯片 8259A，将硬件中断号重新设 置为 0x20 - 0x2f。最后设置 CPU 的控制寄存器 CR0(也称机器状态字)，进入 32 位保护模式运行，并跳 转到位于 system 模块最前面部分的 head.s 程序继续运行。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206102907.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206103200.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206103359.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206103418.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206104727.png)

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206103537.png)

### 1.1.3 head.s

head.s 程序在被编译生成目标文件后会与内核其他程序的目标文件一起被链接成 system 模块，并位 于 system 模块的最前面开始部分。这也就是为什么称其为头部(head)程序的原因。system 模块将被放置 在磁盘上 setup 模块之后开始的扇区中，即从磁盘上第 6 个扇区开始放置。一般情况下 Linux 0.12 内核的 system 模块大约有 120KB 左右，因此在磁盘上大约占 240 个扇区。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105130.png)



![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105150.png)

这段程序实际上处于内存绝对地址 0 处开始的地方。这个程序的功能比较单一。首先它加载各个数 据段寄存器，重新设置中断描述符表 IDT，共 256 项。

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105213.png)

接着程序设置管理内存的分页处理机制，将页目录表放在绝对物理地址 0 开始处(也是本程序所处 的物理内存位置，因此这段程序已执行部分将被覆盖掉)，紧随后面会放置共可寻址 16MB 内存的 4 个 页表，并分别设置它们的表项。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105241.png)

最后，head.s 程序利用返回指令将预先放置在堆栈中的/init/main.c 程序的入口地址弹出，去运行 main() 程序。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105335.png)

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206111530.png" style="zoom:50%;" />

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206105433.png)

总结：引导加载程序 bootsect.S 将 setup.s 代码和 system 模块加载到内存中，并且分别把自己和 setup.s 代码 移动到物理内存 0x90000 和 0x90200 处后，就把执行权交给了 setup 程序。其中 system 模块的首部包含 有 head.s 代码。 

setup 程序的主要作用是利用 ROM BIOS 的中断程序获取机器的一些基本参数，并保存在 0x90000 开始的内存块中，供后面程序使用。同时把 system 模块往下移动到物理地址 0x00000 开始处，这样，system 中的 head.s 代码就处在 0x00000 开始处了。然后加载描述符表基地址到描述符表寄存器中，为进行 32 位保护模式下的运行作好准备。接下来对中断控制硬件进行重新设置，最后通过设置机器控制寄存器 CR0 并跳转到 system 模块的 head.s 代码开始处，使 CPU 进入 32 位保护模式下运行。 

Head.s 代码的主要作用是初步初始化中断描述符表中的 256 项门描述符，检查 A20 地址线是否已经 打开，测试系统是否含有数学协处理器。然后初始化内存页目录表，为内存的分页管理作好准备工作。 最后跳转到 system 模块中的初始化程序 init/main.c 中继续执行。 

## 1.2 初始化

在内核源代码的 init/目录中只有一个 main.c 文件。系统在执行完 boot/head.s 程序后就会将执行权交 给 main.c。 

main.c 程序首先利用前面 setup.s 程序取得的机器参数设置系统的根文件设备号以及一些内存全局变 量。这些内存变量指明了主内存区的开始地址、系统所拥有的内存容量和作为高速缓冲区内存的末端地 址。如果还定义了虚拟盘(RAMDISK)，则主内存区将适当减少。 ![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206113013.png)

然后，内核进行各方面的硬件初始化工作。包括陷阱门、块设备、字符设备和 tty，还包括人工设置 第一个任务(task 0)。待所有初始化工作完成后，程序就设置中断允许标志以开启中断，并切换到任务 0 中运行。到此时，可以说内核已基本完成所有设置工作。接下来内核会通过任务 0 创建几个最初的任 务，运行 shell 程序并显示命令行提示符，从而 Linux 系统进入正常运行阶段。 

**内核初始化程序流程** 

在整个内核完成初始化后，内核将执行控制权切换到了用户模式(任务 0)，也即 CPU 从 0 特权级 切换到了第 3 特权级。此时 main.c 的主程序就工作在任务 0 中。然后系统第一次调用进程创建函数 fork()， 创建出一个用于运行 init()的子进程(通常被称为 init 进程)。系统整个初始化过程见图 7-2 所示。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206163732.png)

由图可见，main.c 程序首先确定如何分配使用系统物理内存，然后调用内核各部分的初始化函数分 别对内存管理、中断处理、块设备和字符设备、进程管理以及硬盘和软盘硬件进行初始化处理。在完成 了这些操作之后，系统各部分已经处于可运行状态。此后程序把自己“手工”移动到任务 0(进程 0)中 运行，并使用 fork()调用首次创建出进程 1(init 进程)，并在其中调用 init()函数。在该函数中程序将继 续进行应用环境的初始化并执行 shell 登录程序。而原进程 0 则会在系统空闲时被调度执行，因此进程 0 通常也被称为 idle 进程。此时进程 0 仅执行 pause()系统调用，并又会调用调度函数。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206165314.png)

## 1.3 Linux系统调用

为了保护系统资源，我们 不能直接让用户程序去访问系统资源。系统调用由0x80中断完成，各个通用寄存器用于传递参数，EAX表示系统调用的接口和结果的返回值。

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206180546.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206181324.png" style="zoom:50%;" />

我们用中断号+系统调用号来表示一个具体的中断。一般int指令表示进入中断，0x80表示系统调用。

下面我们来看下基于int的Linux经典系统调用：

![img](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205114900.png)

第一步是触发中断，比如

```c
int main(){
		fork();
}
```

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206181730.png" style="zoom:50%;" />

当用户调用某个系统调用的时候，实际上执行了一段汇编代码，CPU执行到int 0x80的时候，会保存现场，便于恢复，接着会将特权态切换到内核态，然后CPU会查找中断向量表中第0x80号元素。

第二步是切换堆栈，体来讲，我们需要从用户态的堆栈切换到内核态的堆栈，需要如下步骤：

1. 保存当前的ESP和SS的值；
2. 将ESP和SS的值设置为内核栈的相应值；
   1. 找到当前进程的内核栈（每一个进程都有自己的内核栈）
   2. 在内核中依次压入用户的寄存器SS,ESP,EFLAGS,CS,EIP
   3. 恢复原来的ESP和SS的值（iret）

![img](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205114922.png)

第三步是中断处理程序，

![img](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205114933.png)

![img](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205114948.png)

![img](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191205114958.png)

系统调用(通常称为 syscalls)是 Linux 内核与上层应用程序进行交互通信的唯一接口，参见图 5-4 所示。从对中断机制的说明可知，用户程序通过直接或间接(通过库函数)调用中断 int 0x80，并在 eax 寄存器中指定系统调用功能号，即可使用内核资源，包括系统硬件资源。不过通常应用程序都是使用具 有标准接口定义的 C 函数库中的函数间接地使用内核的系统调用，见图 5-19 所示。 

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206172854.png" style="zoom:50%;" />

在 Linux 内核中，每个系统调用都具有唯一的一个系统调用功能号。内核 0.12 共有 87 个系统调用 功能。这些功能号定义在文件 include/unistd.h 中第 62 行开始处。例如，write 系统调用的功能号是 4，定 义为符号__NR_write。这些系统调用功能号实际上对应于 include/linux/sys.h 中定义的系统调用处理程序 指针数组表 sys_call_table[]中项的索引值。因此 write()系统调用的处理程序指针就位于该数组的项 4 处。 

当我们想在自己的程序中直接使用这些系统调用符号时，需要象下面所示在包括进文件“<unistd.h>” 之前定义符号“__LIBRARY__”。 

当应用程序经过库函数向内核发出一个中断调用int 0x80时，就开始执行一个系统调用。其中寄存 器 eax 中存放着系统调用号，而携带的参数可依次存放在寄存器 ebx、ecx 和 edx 中。因此 Linux 0.12 内 核中用户程序能够向内核最多直接传递三个参数，当然也可以不带参数。处理系统调用中断int 0x80的 过程是程序 kernel/system_call.s 中的 system_call。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206174656.png)

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206174715.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206174746.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206174821.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206174836.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191206175006.png" style="zoom:50%;" />

操作系统实现系统调用的基本过程是：

1. 应用程序调用库函数（API）；
2. API将系统调用号存入EAX，然后通过中断调用使系统进入内核态；
3. 内核中的中断处理函数根据系统调用号，调用对应的内核函数（系统调用）；
4. 系统调用完成相应功能，将返回值存入EAX，返回到中断处理函数；
5. 中断处理函数返回到API中；
6. API将EAX返回给应用程序。

**实现一个系统调用的步骤如下：**

首先，修改 include/linux/sys.h 在sys_call_table数组最后加入sys_foo，并仿照上面给出其他系统调用格式加上
extern rettype sys_foo();

其次，修改include/unistd.h

\#define __NR_foo num

num为接下来使用的系统调用号

然后修改 kernel/system_call.s
nr_system_calls = num,num为在原值加1 即系统调用总数目加1

接着在kernel中添加 foo.c 
若需要支持内核态与用户态数据交互,则包含include/asm/segment.h，其中有put_fs_XXX  get_fs_XXX函数,在foo.c实现系统调用sys_foo()

最后修改kernel的Makefile，将foo.c与内核其它代码编译链接到一起

系统调用用户界面要 

\#define __LIBRARY__

\#include <unistd.h>

_syscallN宏展开系统调用，提供用户态的系统调用接口（参数数目确定具体宏）



# 2. 进程与线程

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191207231228.png)



## 2.1 进程运行轨迹

### 2.1.1  fork.c

fork()系统调用用于创建子进程。Linux 中所有进程都是进程 0(任务 0)的子进程。fork.c 程序是 sys_fork()(在 kernel/sys_call.s 中从 222 行开始)系统调用的辅助处理函数集。它给出了 sys_fork()系统 调用中使用的两个 C 语言函数:find_empty_process()和 copy_process()。还包括进程内存区域验证与内存 分配函数 verify_area()和 copy_mem()。 

copy_process()用于创建并复制进程的代码段和数据段以及环境。在进程复制过程中，工作主要牵涉 到进程数据结构中信息的设置。系统首先为新建进程在主内存区中申请一页内存来存放其任务数据结构 信息，并复制当前进程任务数据结构中的所有内容作为新进程任务数据结构的模板。 

随后程序对已复制的任务数据结构内容进行修改。首先，代码把当前进程设置为新进程的父进程， 清除信号位图并复位新进程各统计值。接着根据当前进程环境设置新进程任务状态段(TSS)中各寄存 器的值。由于创建进程时新进程返回值应为 0，所以需要设置 tss.eax = 0。新建进程内核态堆栈指针 tss.esp0 被设置成新进程任务数据结构所在内存页面的顶端，而堆栈段 tss.ss0 被设置成内核数据段选择符。tss.ldt 被设置为局部表描述符在 GDT 中的索引值。如果当前进程使用了协处理器，则还需要把协处理器的完 整状态保存到新进程的 tss.i387 结构中。 

此后系统设置新任务代码段和数据段的基址和段限长，并复制当前进程内存分页管理的页目录项和 页表项。如果父进程中有文件是打开的，则子进程中相应的文件也是打开着的，因此需要将对应文件的 打开次数增 1。接着在 GDT 中设置新任务的 TSS 和 LDT 描述符项，其中基地址信息指向新进程任务结 构中的 tss 和 ldt。最后再将新任务设置成可运行状态，并向当前进程返回新进程号。 

图 8-13 是内存验证函数 verify_area()中验证内存的起始位置和范围的调整示意图。因为内存写验证 函数 write_verify()需要以内存页面为单位(4096 字节)进行操作，因此在调用 write_verify()之前，需要 把验证的起始位置调整为页面起始位置，同时对验证范围作相应调整。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209114717.png)

上面根据 fork.c 程序中各函数的功能简单描述了 fork()的作用。这里我们从总体上再对其稍加说明。 总的来说 fork()首先会为新进程申请一页内存页用来复制父进程的任务数据结构(也称进程控制块，PCB) 信息，然后会为新进程修改复制的任务数据结构的某些字段值，包括利用系统调用中断发生时逐步压入 堆栈的寄存器信息(即 copy_process()的参数)重新设置任务结构中的 TSS 结构的各字段值，让新进程的状态保持父进程即将进入中断过程前的状态。然后为新进程确定在线性地址空间中的起始位置(nr * 64MB)。对于 CPU 的分段机制，Linux 0.12 的代码段和数据段在线性地址空间中的位置和长度完全相同。 接着系统会为新进程复制父进程的页目录项和页表项。对于Linux 0.12内核来说，所有程序共用一个位 于物理内存开始位置处的页目录表，而新进程的页表则需另行申请一页内存来存放。 

在 fork()的执行过程中，内核并不会立刻为新进程分配代码和数据内存页。新进程将与父进程共同 使用父进程已有的代码和数据内存页面。只有当以后执行过程中如果其中有一个进程以写方式访问内存 时被访问的内存页面才会在写操作前被复制到新申请的内存页面中。 <img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209114858.png" style="zoom:50%;" />

### 2.1.2 **sched.c** 

sched.c 文件是内核中有关任务(进程)调度管理的程序。其中包括几个有关调度的基本函数 (sleep_on()、wakeup()、schedule()等)，以及一些简单的系统调用函数(如 getpid())。系统时钟中断服务 过程中调用的定时函数 do_timer()也被放置在本程序中。另外，为了便于软盘驱动器定时处理编程，Linus 先生也将有关软盘定时操作的几个函数放到了这里。 

调度函数 **schedule()**负责选择系统中下一个要运行的任务(进程)。它首先对所有任务进行检测，唤 醒任何一个已经得到信号的任务。具体方法是针对任务数组中的每个任务，检查其报警定时值 alarm。 如果任务的 alarm 时间已经过期(alarm<jiffies)，则在它的信号位图中设置 SIGALRM 信号，然后清 alarm 值。jiffies 是系统从开机计算起的滴答数(10ms/滴答，在 sched.h 中定义)。如果进程的信号位图中除 去被阻塞的信号外还有其他信号，并且任务处于可中断睡眠状态(TASK_INTERRUPTIBLE)，则置任 务为就绪状态(TASK_RUNNING)。 

随后是调度函数的核心处理部分。这部分代码根据进程的时间片和优先权调度机制，来选择随后要 执行的任务。它首先循环检查任务数组中的所有任务，根据每个就绪态任务剩余执行时间的值 counter， 选取该值最大的一个任务，并利用 switch_to()函数切换到该任务。

 若所有就绪态任务的 counter 值都等于零，表示此刻所有任务的时间片都已经运行完，于是就根据任 务的优先权值 priority，重置每个任务的运行时间片值 counter，再重新循环检查所有任务的执行时间片值。 



自动进入睡眠函数 **sleep_on()和唤醒函数 wake_up()**。这两个函数虽然很短， 却要比 schedule()函数难理解。在看代码之前我们先用图示的方式作一些解释。简单地说，sleep_on()函 数的主要功能是当一个进程(或任务)所请求的资源正被占用或不在内存中时暂时先把该进程切换出去， 放在等待队列中等待一段时间。当切换回来后再继续运行。放入等待队列的方式利用了函数中的 tmp 指 针作为各个正在等待任务的联系。 

函数中共牵涉到对三个任务指针的操作:*p、tmp 和 current。*p 是等待队列头指针，如文件系统内 存 i 节点的 i_wait 指针、内存缓冲操作中的 buffer_wait 指针等;tmp 是在函数堆栈上建立的临时指针， 存储在当前任务内核态堆栈上;current 是当前任务指针。对于这些指针在内存中的变化情况我们可以用 图 8-6 的示意图说明。图中的长条表示内存字节序列。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209113249.png)

当刚进入该函数时，队列头指针*p 指向已经在等待队列中等待的任务结构(进程描述符)。当然， 在系统刚开始执行时，等待队列上无等待任务。因此上图中原等待任务在刚开始时不存在，此时*p 指向 NULL。通过指针操作，在调用调度程序之前，队列头指针指向了当前任务结构，而函数中的临时指针 tmp 指向了原等待任务。在执行调度程序并在本任务被唤醒重新返回执行之前，当前任务指针被指向新 的当前任务，并且 CPU 切换到该新的任务中执行。这样本次 sleep_on()函数的执行使得 tmp 指针指向队 列中队列头指针指向的原等待任务，而队列头指针则指向此次新加入的等待任务，即调用本函数的任务。 

从而通过堆栈上该临时指针 tmp 的链接作用，在几个进程为等待同一资源而多次调用该函数时，内核程 序就隐式地构筑出一个等待队列，参见图 8-7 中的等待队列示意图。图中示出了当向队列头部插入第三 个任务时的情况。从图中我们可以更容易理解 sleep_on()函数的等待队列形成过程。 

![](https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209113512.png)

在把进程插入等待队列后，sleep_on()函数就会调用 schedule()函数去执行别的进程。当进程被唤醒 而重新执行时就会执行后续的语句，把比它早进入等待队列的一个进程唤醒。注意，这里所谓的唤醒并 不是指进程处于执行状态，而是处于可以被调度执行的就绪状态。 

唤醒函数 wake_up()用于把正在等待可用资源的指定任务置为就绪状态。该函数是一个通用唤醒函 数。在有些情况下，例如读取磁盘上的数据块，由于等待队列中的任何一个任务都可能被先唤醒，因此 还需要把被唤醒任务结构的指针置空。这样，在其后进入睡眠的进程被唤醒而又重新执行 sleep_on()时， 就无需唤醒该进程了。 

还有一个函数 interruptible_sleep_on()，它的结构与 sleep_on()的基本类似，只是在进行调度之前是把 当前任务置成了可中断等待状态，并在本任务被唤醒后还需要判断队列上是否有后来的等待任务。若有， 则调度它们先运行。在内核 0.12 开始，这两个函数被合二为一，仅用任务的状态作为参数来区分这两种 情况。 

在阅读本文件的代码时，最好同时参考包含文件 include/linux/sched.h 文件中的注释，以便更全面地 了解内核的调度机理。 

### 2.1.3 exit.c

exit.c 程序主要实现进程终止和退出的相关处理事宜。其中包括进程释放、会话(进程组)终止和程序退出处理函数，以及杀死进程、终止进程、挂起进程等系统调用函数，还包括进程信号发送函数 send_sig() 和通知父进程子进程终止的函数 tell_father()。 

释放进程函数 release()根据指定的任务数据结构(任务描述符)指针，在任务数组中删除指定的任 务指针、释放相关内存页，并立刻让内核重新调度任务运行。 

进程组终止函数 kill_session()用于向会话号与当前进程号相同的进程发送挂断进程的信号。 

系统调用 sys_kill()用于向进程发送任何指定的信号。根据进程号参数 pid 不同数值，该系统调用会 向不同的进程或进程组发送信号。程序注释中列出了在各种不同情况下的处理方式。 

程序退出处理函数do_exit()会在exit系统调用的中断处理程序中被调用。它首先会释放当前进程 的代码段和数据段所占用的内存页面。如果当前进程有子进程，就将子进程的 father 字段置为 1，即把 子进程的父进程改为进程 1(init 进程)。如果该子进程已经处于僵死状态，则向进程 1 发送子进程终止 信号 SIGCHLD。接着关闭当前进程打开的所有文件、释放使用的终端设备、协处理器设备。若当前进程 是进程组的首进程，则还需要终止所有相关进程。随后把当前进程置为僵死状态，设置退出码，并向其 父进程发送子进程终止信号 SIGCHLD。最后让内核重新调度任务运行。 

系统调用 waitpid()用于挂起当前进程，直到 pid 指定的子进程退出(终止)或者收到要求终止该进 程的信号，或者是需要调用一个信号句柄(信号处理程序)。如果 pid 所指的子进程早已退出(已成所谓 的僵死进程)，则本调用将立刻返回。子进程使用的所有资源将释放。该函数的具体操作也要根据其参数 进行不同的处理。详见代码中的相关注释。 

## 2.2 基于内核栈切换的进程切换

**OS-level threads vs Green Threads**

For clarity, I usually say "OS-level threads" or "native threads" instead of "Kernel-level threads" (which I confused with "kernel threads" in my original answer below.) OS-level threads are created and managed by the OS. Most languages have support for them. (C, recent Java, etc) They are extremely hard to use because you are 100% responsible for preventing problems. In some languages, even the native data structures (such as Hashes or Dictionaries) will break without extra locking code.

The opposite of an OS-thread is a [green thread](http://en.wikipedia.org/wiki/Green_thread) that is managed by your language. These threads are given various names depending on the language (coroutines in C, goroutines in Go, fibers in Ruby, etc). These threads only exist inside your language and not in your OS. Because the language chooses context switches (i.e. at the end of a statement), it prevents TONS of subtle race conditions (such as seeing a partially-copied structure, or needing to lock most data structures). The programmer sees "blocking" calls (i.e. `data = file.read()` ), but the language translates it into async calls to the OS. The language then allows *other* green threads to run while waiting for the result.

Green threads are much simpler for the programmer, but their performance varies: If you have a LOT of threads, green threads can be better for both CPU and RAM. On the other hand, most green thread languages can't take advantage of multiple cores. (You can't even buy a single-core computer or phone anymore!). And a bad library can halt the entire language by doing a blocking OS call.

The best of both worlds is to have one OS thread per CPU, and many green threads that are magically moved around onto OS threads. Languages like Go and Erlang can do this.

> system calls and other uses not available to user-level threads

This is only half true. Yes, you can easily cause problems if you call the OS yourself (i.e. do something that's blocking.) But the language usually has replacements, so you don't even notice. These replacements do call the kernel, just slightly differently than you think.

我们本来是需要去学习进程的切换，但是进程的切换包括指令和资源的切换，我们把资源的切换放到内存里面去学习，先来学习指令的切换，也即线程的切换，线程的切换分为用户级线程和内核级线程。

### 2.2.1 用户级线程

正如我们前面所学习到的那样，进程是十分占用资源的，进程 **=** 资源 **+** 指令执行序列。那么我们很容易想到，为了优化性能，我们可不可以：将资源和指令执行分开 ，一个资源 **+** 多个指令执行序列。

所以我们提出了线程的概念：

> 线程的实质是映射表不变而**PC**指针变。

下面我们尝试实现这一一个浏览器：

- 一个线程用来从服务器接收数据

-  一个线程用来显示文本 

- 一个线程用来处理图片**(**如解压缩**)** 
- 一个线程用来显示图片 

我们开始实现这个浏览器：

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209195800.png" style="zoom:25%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209200004.png" style="zoom:25%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209200824.png" style="zoom:25%;" />

发现没有，如果2个线程只有1个栈，就会出现问题，当第二个Yield被执行的时候，应该204出栈，但是这个时候确是404出栈，这个时候出现了问题。

<img src="../../../Library/Application Support/typora-user-images/image-20191209201200838.png" alt="image-20191209201200838" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209201329.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209201614.png" style="zoom:50%;" />

### 2.2.2 内核级线程的样子

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209203045.png" style="zoom:50%;" />

<img src="../../../Library/Application Support/typora-user-images/image-20191209203117609.png" alt="image-20191209203117609" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209203736.png" style="zoom:50%;" />



<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191209203759.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191210105922.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191210110911.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191210111052.png" style="zoom:50%;" />

<img src="https://raw.githubusercontent.com/Anapodoton/ImageHost/master/img/20191210111121.png" style="zoom:50%;" />

### 2.2.3 内核级线程的实现

<img src="https://i.loli.net/2019/12/11/8PjDQ5TVEyldAts.png" style="zoom:50%;" />

<img src="https://i.loli.net/2019/12/11/wZ2SG6R3xqp9ujb.png" style="zoom:50%;" />

<img src="../../../Library/Application Support/typora-user-images/image-20191211182238864.png" alt="image-20191211182238864" style="zoom:50%;" />



<img src="https://i.loli.net/2019/12/11/DZrzSAYnhEIFMVX.png" style="zoom:50%;" />

<img src="https://i.loli.net/2019/12/11/SOGU3zcliTvNAbJ.png" style="zoom:50%;" />

<img src="https://i.loli.net/2019/12/11/SAtNu8qBMseVw7L.png" style="zoom:50%;" />

<img src="https://i.loli.net/2019/12/11/9PZfNECjhoDgBuT.png" style="zoom:50%;" />



<img src="https://i.loli.net/2019/12/11/VirPey1TlK4kng3.png" style="zoom:50%;" />

![](https://i.loli.net/2019/12/11/gd38lwFOroyJ7qN.png)

![](https://i.loli.net/2019/12/11/tBuDMKspkzVP2Tn.png)

![](https://i.loli.net/2019/12/11/Bo6XbTJ5favYiEH.png)

![](https://i.loli.net/2019/12/11/3WmT9hOlHP2QZ1R.png)

到这里，我们来总结下，CPU运转后，由于有些进程比较耗时，所以我们引入了多进程。如果我们只用一个栈来处理进程的切换，将会造成混乱，所以我们引入了2个栈。有些调用是在内核态的，所以我们必须考虑用户态和内核态的切换，我们引入了切换的5段论。

下面我们想实现一段程序，来交替打印A和B：

```c
main(){
if(!fork()){while(1)printf(“A”);} 
if(!fork()){while(1)printf(“B”);}
wait();
}
```

当我们调用fork的时候，将会通过INT指令进入内核，执行sys_fork,调用copy_process,我们将通过ret指令返回到父进程，iret指令返回到子进程。然后再次执行fork，执行了**schedule** ，然后进行了**switch_to**切换。那么A什么时候可以和B进行交替打印，答案是时钟中断。

## 2.2 信号量的实现和应用

### 2.2.1 进程同步与信号量

**Processes Synchronization and** **Semaphore** 

![](https://i.loli.net/2019/12/12/UBbxaHkZQ3NVXny.png)

注意，这个程序是有问题的，如果，在生产者访问的时候，别切走了，切回来之后，counter被修改了，但是生产者还不知道。

![image-20191212194535873](https://tva1.sinaimg.cn/large/006tNbRwly1g9u6g4o6t0j311g0nggru.jpg)

![image-20191212194939432](https://tva1.sinaimg.cn/large/006tNbRwly1g9u6kbw0ooj312g0ssait.jpg)

![image-20191212195510782](https://tva1.sinaimg.cn/large/006tNbRwly1g9u6q2qzk4j310w0nswkl.jpg)

![image-20191212195730028](https://tva1.sinaimg.cn/large/006tNbRwly1g9u6sii3amj31cs0oy4ny.jpg)

**empty表示空闲缓冲区的个数，full表示已经生产的个数,multx表示互斥信号：**

![image-20191212200905562](https://tva1.sinaimg.cn/large/006tNbRwly1g9u74k1x6yj311e0neafj.jpg)

### 2.2.2 信号量临界区保护

**靠临界区保护信号量，靠信号量实现进程的同步。**

**Critical Section** 

温故而知新:什么是信号量? 通过对这个量的访问和修改，让大 家有序推进。哪里还有问题吗? 

empty=-1，表示已经有一个进程在睡眠了。

![image-20191212202434812](https://tva1.sinaimg.cn/large/006tNbRwly1g9u7kop889j30po0jowi6.jpg)

![image-20191212202747672](https://tva1.sinaimg.cn/large/006tNbRwly1g9u7o0vh87j30ti0nidkp.jpg)

![image-20191212202829655](https://tva1.sinaimg.cn/large/006tNbRwly1g9u7orriy2j31480n8grz.jpg)

![](https://tva1.sinaimg.cn/large/006tNbRwly1g9u7rrexoaj30xo0jy0xd.jpg)

**进入临界区的方法：**

轮换法(也称值日法)，满足互斥，但是不满足有空让进，比如，P0进入后，turn=1，然后由于某种原因，一直在等待，这个时候，P2也无法进入。

![image-20191212203347425](https://tva1.sinaimg.cn/large/006tNbRwly1g9u7u9mh5kj30zy0g6whg.jpg)

然后我们引入了标记法，发现虽然满足了，互斥，有空让进，但是不满足有限等待。

![image-20191212205222610](https://tva1.sinaimg.cn/large/006tNbRwly1g9u8dlhh1dj30ho0usdjv.jpg)

然后我们又引入了非对称标记：

![image-20191212205535763](https://tva1.sinaimg.cn/large/006tNbRwly1g9u8gxtc4ej30ny0hygp8.jpg)

flag[1]=true,turn=1,表示1要进入临界区，并且轮到1进入。

![image-20191212205649568](https://tva1.sinaimg.cn/large/006tNbRwly1g9u8i7pb1dj30k00u242h.jpg)

下面我们只是讨论了2个进程，那么多个进程怎么办呢？

![image-20191213105159256](https://tva1.sinaimg.cn/large/006tNbRwly1g9uwn7i9v5j31640qctvv.jpg)



![image-20191213105357556](https://tva1.sinaimg.cn/large/006tNbRwly1g9uwp94lelj31bc0qenho.jpg)

面包店算法是在是太复杂了，我们尝试另外的解法，回想一下，我们只允许一个进程进入，这句话意味着什么呢？意味着另一个进程只有被调度才可以进入临界区，那么我们只要阻止时钟中断就好了。即CPU不会在调度。

![image-20191213111729120](https://tva1.sinaimg.cn/large/006tNbRwly1g9uxdquz7aj31a00q6wzf.jpg)

但是多CPU不行，因为我们没法关掉所有CPU的中断。

接下来我们引入了硬件原子指令法：

![image-20191213111052435](https://tva1.sinaimg.cn/large/006tNbRwly1g9ux6v8u7dj314e0rm79r.jpg)

### 2.2.3 信号量的代码实现 

**Coding Semaphore** 

![image-20191213113025775](https://tva1.sinaimg.cn/large/006tNbRwly1g9uxr7xl8uj31aa0pqtxo.jpg)

![image-20191213113358605](https://tva1.sinaimg.cn/large/006tNbRwly1g9uxuwbkp0j31920qynp1.jpg)

![image-20191213113605982](https://tva1.sinaimg.cn/large/006tNbRwly1g9uxx3iru3j31240qcdtz.jpg)

![image-20191213113450967](https://tva1.sinaimg.cn/large/006tNbRwly1g9uxvu4nzpj31ca0q4kd4.jpg)

### 2.2.4 死锁处理



