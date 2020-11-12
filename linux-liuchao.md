# 前言

![image-20201102184241461](https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201102184241.png)

# 1. Linux操作系统综述

## 4. 常见命令

CentOS 体系是rpm，Ubuntu 体系是deb。

```
# 安装
rpm -i jdk-XXX_linux-x64_bin.rpm
dpkg -i jdk-XXX_linux-x64_bin.deb
# 查询
rpm -qa
dpkg -l
# 删除
rpm -e
dpkg -r
```

使用软件管家：

```
# 搜索
yum search jdk
apt-cache search jdk
# 卸载
yum erase java-11-openjdk.x86_64
apt-get purge openjdk-9-jdk
```

注意，如果我们使用wget来下载，那么我们需要配置环境变量,如：

```
export JAVA_HOME=/root/jdk-XXX_linux-x64
export PATH=$JAVA_HOME/bin:$PATH
```

注意，nohup命令（ no hang up）：

```
nohup command >out.file 2>&1 &
```

“1”表示文件描述符 1，表示标准输出，“2”表示文件描述符 2，意思是标准错误输出，“2>&1”表示标准输出和错误输出合并了。合并到 out.file 里。

关闭程序：

```
ps -ef |grep 关键字 |awk '{print $2}'|xargs kill -9
```

在ubuntu中，可以通过systemctl start mysql启动mysql，通过systemctl enable mysql设置开机启动。

![image-20201102191102894](https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201102191102.png)

## 5. 常见系统调用

fork：当父进程调用 fork 创建进程的时候，子进程将各个子系统为父进程创建的数据结构也全部拷贝了一份，甚至连程序代码也是拷贝过来的。父进程返回子进程的id，子进程返回0。

waitpid：父进程查看子进程的状态。

![image-20201102203412783](https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201102203505.png)

掌握strace命令：可以显示某个命令执行的详细信息。

```shell
root@chin:~# strace ls
execve("/bin/ls", ["ls"], [/* 21 vars */]) = 0
brk(NULL)                               = 0x158e000
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
open("/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=34090, ...}) = 0
mmap(NULL, 34090, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f0b31461000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libselinux.so.1", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260Z\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=130224, ...}) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f0b31460000
mmap(NULL, 2234080, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f0b31023000
mprotect(0x7f0b31042000, 2093056, PROT_NONE) = 0
mmap(0x7f0b31241000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e000) = 0x7f0b31241000
mmap(0x7f0b31243000, 5856, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f0b31243000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0P\t\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1868984, ...}) = 0
mmap(NULL, 3971488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f0b30c59000
mprotect(0x7f0b30e19000, 2097152, PROT_NONE) = 0
mmap(0x7f0b31019000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1c0000) = 0x7f0b31019000
mmap(0x7f0b3101f000, 14752, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f0b3101f000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libpcre.so.3", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0000\25\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=456632, ...}) = 0
mmap(NULL, 2552072, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f0b309e9000
mprotect(0x7f0b30a57000, 2097152, PROT_NONE) = 0
mmap(0x7f0b30c57000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x6e000) = 0x7f0b30c57000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libdl.so.2", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\240\r\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=14608, ...}) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f0b3145f000
mmap(NULL, 2109680, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f0b307e5000
mprotect(0x7f0b307e8000, 2093056, PROT_NONE) = 0
mmap(0x7f0b309e7000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2000) = 0x7f0b309e7000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libpthread.so.0", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260`\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=138696, ...}) = 0
mmap(NULL, 2212904, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f0b305c8000
mprotect(0x7f0b305e0000, 2093056, PROT_NONE) = 0
mmap(0x7f0b307df000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x17000) = 0x7f0b307df000
mmap(0x7f0b307e1000, 13352, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f0b307e1000
close(3)                                = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f0b3145e000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f0b3145c000
arch_prctl(ARCH_SET_FS, 0x7f0b3145c800) = 0
mprotect(0x7f0b31019000, 16384, PROT_READ) = 0
mprotect(0x7f0b307df000, 4096, PROT_READ) = 0
mprotect(0x7f0b309e7000, 4096, PROT_READ) = 0
mprotect(0x7f0b30c57000, 4096, PROT_READ) = 0
mprotect(0x7f0b31241000, 4096, PROT_READ) = 0
mprotect(0x61d000, 4096, PROT_READ)     = 0
mprotect(0x7f0b3146a000, 4096, PROT_READ) = 0
munmap(0x7f0b31461000, 34090)           = 0
set_tid_address(0x7f0b3145cad0)         = 24076
set_robust_list(0x7f0b3145cae0, 24)     = 0
rt_sigaction(SIGRTMIN, {0x7f0b305cdb50, [], SA_RESTORER|SA_SIGINFO, 0x7f0b305d9390}, NULL, 8) = 0
rt_sigaction(SIGRT_1, {0x7f0b305cdbe0, [], SA_RESTORER|SA_RESTART|SA_SIGINFO, 0x7f0b305d9390}, NULL, 8) = 0
rt_sigprocmask(SIG_UNBLOCK, [RTMIN RT_1], NULL, 8) = 0
getrlimit(RLIMIT_STACK, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
statfs("/sys/fs/selinux", 0x7fff2c537260) = -1 ENOENT (No such file or directory)
statfs("/selinux", 0x7fff2c537260)      = -1 ENOENT (No such file or directory)
brk(NULL)                               = 0x158e000
brk(0x15af000)                          = 0x15af000
open("/proc/filesystems", O_RDONLY)     = 3
fstat(3, {st_mode=S_IFREG|0444, st_size=0, ...}) = 0
read(3, "nodev\tsysfs\nnodev\trootfs\nnodev\tr"..., 1024) = 403
read(3, "", 1024)                       = 0
close(3)                                = 0
open("/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=2981280, ...}) = 0
mmap(NULL, 2981280, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f0b302f0000
close(3)                                = 0
ioctl(1, TCGETS, {B9600 opost isig icanon echo ...}) = 0
ioctl(1, TIOCGWINSZ, {ws_row=24, ws_col=80, ws_xpixel=880, ws_ypixel=552}) = 0
open(".", O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFDIR|0700, st_size=4096, ...}) = 0
getdents(3, /* 35 entries */, 32768)    = 1152
getdents(3, /* 0 entries */, 32768)     = 0
close(3)                                = 0
fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(136, 2), ...}) = 0
write(1, "block_chain.sql  emqx-ubuntu16.0"..., 80block_chain.sql  emqx-ubuntu16.04-v3.2.3_amd64.deb  quality-certification-chain
) = 80
write(1, "blockimg\t go\t\t\t\t    terminal\n", 29blockimg	 go		    terminal
) = 29
write(1, "demo-bak.jar\t gopath\t\t\t\t    work"..., 33demo-bak.jar	 gopath    work
) = 33
write(1, "demo.jar\t nohup.out\n", 20demo.jar	 nohup.out
)  = 20
close(1)                                = 0
close(2)                                = 0
exit_group(0)                           = ?
+++ exited with 0 +++
```

# 2. 系统初始化

## 6. x86架构

<img src="https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201102204941.png" alt="image-20201102204603089" style="zoom:50%;" />



**CPU 和内存是如何配合工作的：**

CPU 其实也不是单纯的一块，它包括三个部分，运算单元、数据单元和控制单元。

**运算单元**只管算，例如做加法、做位移等等。但是，它不知道应该算哪些数据，运算结果应 该放在哪里。

运算单元计算的数据如果每次都要经过总线，到内存里面现拿，这样就太慢了，所以就有了 **数据单元**。数据单元包括 CPU 内部的缓存和寄存器组，空间很小，但是速度飞快，可以暂 时存放数据和运算结果。

有了放数据的地方，也有了算的地方，还需要有个指挥到底做什么运算的地方，这就是**控制单元**。控制单元是一个统一的指挥中心，它可以获得下一条指令，然后执行这条指令。这个 指令会指导运算单元取出数据单元中的某几个数据，计算出个结果，然后放在数据单元的某个地方。

总线上主要有两类 数据，一个是地址数据，也就是我想拿内存中哪个位置的数据，这类总线叫**地址总线** (Address Bus);另一类是真正的数据，这类总线叫**数据总线**(Data Bus)。

<img src="https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201102210650.png" alt="image-20201102210648273" style="zoom:50%;" />

[x86参考](http://www.cs.virginia.edu/~evans/cs216/guides/x86.html)

IP 寄存器就是指令指针寄存器(Instruction Pointer Register)，指向代码段中下一条指令 的位置。CPU 会根据它来不断地将指令从内存的代码段中，加载到 CPU 的指令队列中，然 后交给运算单元去执行。

CS 就是代码段寄存器(Code Segment Register)，通过它可以找到代码在内存中的位置;

DS 是数据段的寄存器，通过它可以找到数据在内存中的位置。

SS 是栈寄存器(Stack Register)。

## 7. **从**BIOS到bootloader

![image-20201103164244982](https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201103164249.png)



## 8. 内核初始化

![image-20201103194017368](https://cdn.jsdelivr.net/gh/haojunsheng/ImageHost/img/20201103194211.png)

进程切换：

![image-20201103170135417](https://raw.githubusercontent.com/haojunsheng/ImageHost/master/img/20201103170648.png)

## 9. 系统调用

![image-20201103194409125](https://cdn.jsdelivr.net/gh/haojunsheng/ImageHost/img/20201103194409.png)













