# 字符显示的控制

## 实验目的

- 加深对操作系统设备管理基本原理的认识，实践键盘中断、扫描码等概念；
- 通过实践掌握Linux 0.11对键盘终端和显示器终端的处理过程。

## 实验内容

本实验的基本内容是修改Linux 0.11的终端设备处理代码，对键盘输入和字符显示进行非常规的控制。 在初始状态，一切如常。用户按一次F12后，把应用程序向终端输出所有字母都替换为“*”。用户再按一次F12，又恢复正常。第三次按F12，再进行输出替换。依此类推。 以ls命令为例： 正常情况：

```
# ls
hello.c hello.o hello
```

第一次按F12，然后输入ls：

```
# **
*****.* *****.* *****
```

第二次按F12，然后输入ls：

```
# ls
hello.c hello.o hello
```

第三次按F12，然后输入ls：

```
# **
*****.* *****.* *****
```

## 实验报告

这个实验本身的代码部分是很容易的，因为要修改的地方并不多。只要弄清楚，字符从键盘输入到显示在终端上的整个过程就知道如何控制键盘输入的回显问题。

### 一、键盘工作流程

**1、键盘中断初始化**

键盘中断的初始化在boot/main.c文件的init()函数中，在这个函数中会调用tty_init()函数对显卡的变量等进行设置（如：video_size_row，vide_mem_start，video_port_reg，video_port_val，video_mem_end等等），并对键盘中断0x21设置，将中断过程指向keyboard_interrupt函数（这是通过set_trap_gate(0x21, &keyboard_interrupt)语句来实现的）。

**2、键盘中断发生**

当键盘中断发生时，会取出键盘的扫描码，放在al寄存器中，然后查key_table表进行扫描码处理，key_table表中定义了相关所有扫描码对应键的处理函数(如do_self，func，alt，ctrl，none等)，比如f1-f12键的则要先运行一段处理函数func，调用sched.c中定义的show_stat函数显示当前进程情况，默认情况下0.11中所有的功能键都进行这样的动作，然后再进行下一步处理，将控制键以及功能键进行转义，比如ctrl_a将会被转义为^a，f1会被转义为esc[[A，f2被转义为esc[[B，这也是有时按功能键除了显示进程信息外，也会显示一个字符A-L的原因，想来应该是一个bug吧。而其他键也经过类似处理，处理完毕后将跳到put_queue，将扫描码放在键盘输入队列中，当然如果没有对应的扫描码被找到，则会通过none标签直接退回，不会被放入队列。然后再调用do_tty_interrupt函数进行最后的处理。do_tty_interrupt直接调用copy_to_cooked函数对队列中的字符进行判断处理，最后调用con_write函数将其输出到显卡内存。在此同时，也会将字符放入辅助队列中，以备缓冲区读写程序使用。

> keyboard.S主要负责将用户键入的字符放入`read_q`缓冲队列中; 另一个是屏幕显示处理程序`console.c`, 用于从`write_q`队列中取出字符并显示在屏幕上． ![流程图](https://tva1.sinaimg.cn/large/006tNbRwly1g9ygb5bx57j30ov0dqdhl.jpg)

从图中可以看出:用户键入的字符先存放在`read_q`队列中, 经过`copy_to_cook`函数之后, 放入`secondary`,`tty_write`写入`write_q`中, 最后调用con_write()函数输出到显示器上.

> 1. 我们可以在copy_to_cooked()函数做手脚, 这样放入`write_q`队列中的所有字符都是星号, 这种与题意不是相符.因为这种方法会将原字符彻底修改成'*', 而不仅仅修改显示的字符, 这样以来, 系统会找不到用户输入的命令.
> 2. 我们可以在con_write()函数做手脚, 使其显示出来的字符是'*';

### 二、字符输出流程

如果是将字符输出，则会通过以下流程来进行。无论是输出字符到屏幕上还是文件中，系统最终都会调用write函数来进行，而write函数则会调用sys_write系统调用来实现字符输出，如果是输出到屏幕上，则会调用tty_write函数，而tty_write最终也会调用con_write函数将字符输出；如果是输出到文件中，则会调用file_write函数直接将字符输出到指定的文件缓冲区中。

通过以上分析，会发现如果要控制字符输出到屏幕的结果，无论是键盘输入的回显，还是程序的字符输出，那只需要修改con_write最终写到显存中的字符就可以了。但如果要控制字符输出到文件中，则需要修改file_write函数中输出到文件缓冲区中的字符即可。

### 三、代码修改部分

**1、添加f12功能键的处理**

在此次实验中，要添加一个全局的开关变量来跟踪f12被按后当前字符输出的过滤状态。如果当前是过滤状态，则开关变量关，否则开头变量为开的状态。当开头变量为开的状态时，字符的所有输出就会被过滤成*号。选择全局变量的加入位置，一般而言sched.c中会是一个方便的位置.变量的定义可以放在sched.h文件中，而变量的初始化则放在sched.c文件中即可。同时开头变量的修改可以在sched.c文件中定义一个函数来实现。这里要注意的是，在sched.h文件中只能定义变量，不能进行初始化，否则会在编译时收到多处定义的错误。开关修改函数被调用的时机则应该是f12键被按下时，所以要在键盘中断函数中添加相应的代码，判断如果是f12被按下，则调用开关修改函数。系统在处理功能键时，会调用func过程来进行处理，而且系统自身对功能键也附加了一个显示进程信息的功能，所以只需要对这部分代码进行修改，加入f12的处理代码即可。

这部分修改的代码如下：

```
//sched.h
int f12_state;
//sched.c
//初始化开关变量
f12_state=0;
void switch_f12(void)
{
    if(f12_state==1)
        f12_state=0;
    else
        f12_state=1;
}
//keyboard.S文件中
//func
func:
    subb $0x3B,%al    /*f1 - f10*/
    jb end_func
    cmpb $9,%al
    jbe ok_stat
    subb $18,%al    /* f11 - f12 */
    cmpb $10,%al    /*f11*/
    cmpb $11,%al        /*f12*/
    ja end_func

ok_f12:
    pushl %eax
    pushl %ecx
    pushl %edx
    call switch_f12
    popl %edx
    popl %ecx
    popl %eax
    jmp ok_func

ok_stat:
    pushl %eax
    pushl %ecx
    pushl %edx
    call show_stat
    popl %edx
    popl %ecx
    popl %eax

ok_func:
    cmpl $4,%ecx        /* check that there is enough room */
    jl end_func

ok_put_queue:
    movl func_table(,%eax,4),%eax
    xorl %ebx,%ebx
    jmp put_queue
end_func:
    ret
```

**2、字符输出的控制**

**2．1、输出到屏幕上的字符**

要控制输出到屏幕上的字符，只要将con_write函数中输出的字符修改为*号即可。因为无论是键盘的按键回显还是程序中输出字符，最终都会转到con_write中进行最终的显存写字符。当然，也可以在tty_write函数中将要放入write_q写队列中的字符修改为*号。但这种方法会将write_q队列中的原字符彻底修改成*号，而不是仅仅修改显示的字符。 在本次实验中，本人是个性con_write函数来实现的。代码如下：

```
//console.c
//con_write函数
void con_write(struct tty_struct * tty)
{
    int nr;
    char c;

    nr = CHARS(tty->write_q);
    while (nr--) {
        GETCH(tty->write_q,c);
        switch(state) {
            case 0:
                if (c>31 && c<127) {
                    if (x>=video_num_columns) {
                        x -= video_num_columns;
                        pos -= video_size_row;
                        lf();
                    }
                    /*melon 2015-9-13*/

                    if(f12_state==1)
                        c='*';

                    /*melon*/
                    __asm__("movb attr,%%ah\n\t"
                        "movw %%ax,%1\n\t"
                        ::"a" (c),"m" (*(short *)pos)
                        );
                    pos += 2;
                    x++;
……
```

**2．2、文件中字符的输出控制**

控制文件中字符输出的结果，需要修改file_write函数中关于写字符到输出缓冲区部分代码来实现。代码如下：

```
//file_dev.c
int file_write(struct m_inode * inode, struct file * filp, char * buf, int count)
{
    off_t pos;
    int block,c;
    struct buffer_head * bh;
    char * p;
    int i=0;

    char tmpChar;

/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
    if (filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    while (i<count) {
        if (!(block = create_block(inode,pos/BLOCK_SIZE)))
            break;
        if (!(bh=bread(inode->i_dev,block)))
            break;
        c = pos % BLOCK_SIZE;
        p = c + bh->b_data;
        bh->b_dirt = 1;
        c = BLOCK_SIZE-c;
        if (c > count-i) c = count-i;
        pos += c;
        if (pos > inode->i_size) {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        i += c;
        while (c-->0)
        {
            /*melon 2015-9-13*/
            tmpChar=get_fs_byte(buf);
            if(f12_state==1)  /* && tmpChar>31 && buf<127)*/
            {
                *(p++)='*';
                buf++;
                /*get_fs_byte(buf++);*/
            }
            else
                *(p++)=get_fs_byte(buf++);
            /*melon edited*/
            //*(p++) = get_fs_byte(buf++);
        }
        brelse(bh);
    }
    inode->i_mtime = CURRENT_TIME;
    if (!(filp->f_flags & O_APPEND)) {
        filp->f_pos = pos;
        inode->i_ctime = CURRENT_TIME;
    }
    return (i?i:-1);
}
```

然后直接make，在linux0.11中分别输入ls,然后按下f12,再次输入ls。

### 四、完成实验后，在实验报告中回答如下问题：

- 1、在原始代码中，按下F12，中断响应后，中断服务程序会调用func？它实现的是什么功能？
- 原始代码中对f1 – f12功能键按下后都会显示当前进程信息。这个是通过show_stat函数来完成的。
- 2、在你的实现中，是否把向文件输出的字符也过滤了？如果是，那么怎么能只过滤向终端输出的字符？如果不是，那么怎么能把向文件输出的字符也一并进行过滤？
- 在我的实现中没有将文件输出字符和显示字符一并过滤。只过滤向终端输出的字符是通过con_write函数的修改来实现的。过滤向文件输出的字符则通过修改file_write函数来实现。

### 五、本人对相关代码的分析及注释

这里涉及到键盘处理的几个数据结构，最主要的就是tty_table结构、tty_table数组，0.11中定义了三个第一个是用来键盘输入的，其他两个是用来处理串口输入的。其结构如下：

```
struct tty_struct {
    struct termios termios; //特殊字符数组结构，如在当前系统中定义intr=^C
    int pgrp;                //所属进程组
    int stopped;            //停止标志
    void (*write)(struct tty_struct * tty);        //写函数指针，系统中定义的键盘写函数为con_write
    struct tty_queue read_q;                    //读队列
    struct tty_queue write_q;                    //写队列
    struct tty_queue secondary;                    //辅助队列
    };

struct tty_struct tty_table[] = {
    {
        //第一个定义的是键盘结构
        {ICRNL,        /* change incoming CR to NL */
        OPOST|ONLCR,    /* change outgoing NL to CRNL */
        0,
        ISIG | ICANON | ECHO | ECHOCTL | ECHOKE,
        0,        /* console termio */
        INIT_C_CC},        //定义特殊字符数组
        0,            /* initial pgrp */
        0,            /* initial stopped */
        con_write,        //写函数指针
        {0,0,0,0,""},        /* console read-queue */
        {0,0,0,0,""},        /* console write-queue */
        {0,0,0,0,""}        /* console secondary queue */
    },
    {
        //第二个及第三个是串口输入使用的结构
        {0, /* no translation */
        0,  /* no translation */
        B2400 | CS8,
        0,
        0,
        INIT_C_CC},
        0,
        0,
        rs_write,
        {0x3f8,0,0,0,""},        /* rs 1 */
        {0x3f8,0,0,0,""},
        {0,0,0,0,""}
    },{
        {0, /* no translation */
        0,  /* no translation */
        B2400 | CS8,
        0,
        0,
        INIT_C_CC},
        0,
        0,
        rs_write,
        {0x2f8,0,0,0,""},        /* rs 2 */
        {0x2f8,0,0,0,""},
        {0,0,0,0,""}
    }
};

//定义的输入读、写及辅助队列。
struct tty_queue {
    unsigned long data;        //字符行数，这个可以在处理辅助队列中看到如果有回车，则行数加1
    unsigned long head;        //头指针 - 读
    unsigned long tail;        //尾指针 - 写
    struct task_struct * proc_list;    //等待进程队列
    char buf[TTY_BUF_SIZE];    //队列缓冲区数组
};

//用来保存读、写队列的指针。
struct tty_queue * table_list[]={
    &tty_table[0].read_q, &tty_table[0].write_q,
    &tty_table[1].read_q, &tty_table[1].write_q,
    &tty_table[2].read_q, &tty_table[2].write_q
    };

//下面通过功能键f12的按下处理流程来看一下键盘中断处理的整个工作过程。

keyboard_interrupt:    //1、f12键被按下，keyborad_interrupt函数被调用。
    pushl %eax        //2、对现场进行保护
    pushl %ebx
    pushl %ecx
    pushl %edx
    push %ds
    push %es
    movl $0x10,%eax    //3、进入内核，这个和0x80的处理方式基本一致，中断在内核中进行
    mov %ax,%ds
    mov %ax,%es
    xor %al,%al        /* %eax is scan code */
    inb $0x60,%al    //4、取键盘扫描码
    cmpb $0xe0,%al    //对扫描进行判断处理，看是否为扩展字符
    je set_e0
    cmpb $0xe1,%al
    je set_e1
    call key_table(,%eax,4)    //5、对扫描码进行处理，这里调用func
    movb $0,e0
e0_e1:    inb $0x61,%al
    jmp 1f
1:    jmp 1f
1:    orb $0x80,%al        //此语句禁止键盘工作
    jmp 1f
1:    jmp 1f
1:    outb %al,$0x61
    jmp 1f
1:    jmp 1f
1:    andb $0x7F,%al
    outb %al,$0x61
    movb $0x20,%al
    outb %al,$0x20        //以上几句都用来处理键盘是否可以继续工作，到些键盘可以继续接收按键
    pushl $0                //此处将do_tty_interrupt函数的参数入栈，这个参数会传递给copy_to_cooked(tty_table+tty)
    call do_tty_interrupt    //7、进行中断后处理函数，即处理字符进行转义然后处理光标等事情
    addl $4,%esp            //调整栈，然后退出中断处理
    pop %es
    pop %ds
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret
set_e0:    movb $1,e0
    jmp e0_e1
set_e1:    movb $2,e0
    jmp e0_e1

//功能键对应的字符序列
func_table:
    .long 0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b
    .long 0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b
    .long 0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b


//功能键处理过程
func:                    //5、对扫描码进行处理
    pushl %eax            //调用show_stat函数显示进程信息
    pushl %ecx
    pushl %edx
    call show_stat
    popl %edx
    popl %ecx
    popl %eax

    subb $0x3B,%al    /*f1 - f10*///判断是否为功能键f1-f10，将扫描码进行减0x3B，
    jb end_func
    cmpb $9,%al
    jbe ok_func        /*f1 - f10*/

    subb $18,%al    /* f11 - f12 *///判断是否为功能键f11-f12
    cmpb $10,%al    /*f11*/
    jb end_func
    cmpb $11,%al        /*f12*/
    ja end_func

ok_func:
    cmpl $4,%ecx        /* check that there is enough room *///这里应该是如果前面有键进来，可能会导致缓冲区空闲位置不足4位，如果是，则直接退出？其实没明白。
    jl end_func

ok_put_queue:
    movl func_table(,%eax,4),%eax    //将转义后的功能键字符放入eax中，此时eax中的值为0x4c5b5b1b即L[[esc
    xorl %ebx,%ebx
    jmp put_queue                    //6、将转义后的字符放入读队列中
end_func:
    ret

//将字符放入缓冲区
put_queue:                        #6、将转义后的字符放入读队列中
    pushl %ecx
    pushl %edx
    movl table_list,%edx        # read-queue for console 取缓冲区结构首地址，这里意思是数组名称就是第一个元素的地址，而这里的第一个元素就是键盘读队列地址
    movl head(%edx),%ecx        #head=4,buf=16,tail=8,这些都是键盘读队列元素偏移
1:    movb %al,buf(%edx,%ecx)        #将一个字符放在头指针处，即缓冲区尾部
    incl %ecx                    #头指针前移
    andl $size-1,%ecx            #size=1024，此句即解决头指针是否超出缓冲区大小，如果超出，则回到第0位。
    cmpl tail(%edx),%ecx        # buffer full - discard everything缓冲区满了吗？如果满，则后续字符不放，直接退出
    je 3f
    shrdl $8,%ebx,%eax            #eax向右移n位,其“空出”的高位由ebx的低n位来填补,但ebx自己不改变，此前xorl %ebx,%ebx语句
                                #将ebx中值变为0，所以下面才会判断eax中是否为0，如果为0则说明eax中的字符全部处理完毕，
                                #如果不是0，则此操作会将下一个字符移到低8位，即al中。
    je 2f
    shrl $8,%ebx                #将ebx右移8位，继续处理，此处似乎不用移位也可以吧，因为ebx中为0，而且处理过程中也没有改变ebx中的值。
    jmp 1b                        #继续处理剩余字符，
2:    movl %ecx,head(%edx)        #修改头指针，
    movl proc_list(%edx),%ecx    #proc_list=12，即进程等待队列指针，看是否有进程在等读队列。
    testl %ecx,%ecx
    je 3f
    movl $0,(%ecx)                #如果有进程在等待，则此语句会唤醒这个进程。
                                #struct task_struct {
                                #/* these are hardcoded - don't touch */
                                #    long state;    /* -1 unrunnable, 0 runnable, >0 stopped */
3:    popl %edx
    popl %ecx
    ret

//键盘中断后期处理函数    
void do_tty_interrupt(int tty)    //7、进行中断后处理函数，即处理字符进行转义然后处理光标等事情
{
    copy_to_cooked(tty_table+tty);    //8、调用真正的处理函数
}

//8、将读队列中的字符放入辅助队列及如果回显状态，则也放入写队列，并调用显示函数con_write进行显示。
void copy_to_cooked(struct tty_struct * tty)
{
    signed char s;
    signed char c;

    while (!EMPTY(tty->read_q) && !FULL(tty->secondary)) {
        GETCH(tty->read_q,c);

        if (c==13)                                        //处理回车换行
            if (I_CRNL(tty))
                c=10;
            else if (I_NOCR(tty))
                continue;
            else ;

        else if (c==10 && I_NLCR(tty))
            c=13;
        if (I_UCLC(tty))                                //处理大写转小写
            c=tolower(c);

        if (L_CANON(tty)) {                                //如果本地集中模式置位，则进行处理
            if (c==KILL_CHAR(tty)) {
                /* deal with killing the input line */
                while(!(EMPTY(tty->secondary) ||
                        (c=LAST(tty->secondary))==10 ||
                        c==EOF_CHAR(tty))) {
                    if (L_ECHO(tty)) {
                        if (c<32)
                            PUTCH(127,tty->write_q);
                        PUTCH(127,tty->write_q);
                        tty->write(tty);
                    }
                    DEC(tty->secondary.head);
                }
                continue;
            }
            if (c==ERASE_CHAR(tty)) {
                if (EMPTY(tty->secondary) ||
                   (c=LAST(tty->secondary))==10 ||
                   c==EOF_CHAR(tty))
                    continue;
                if (L_ECHO(tty)) {                        //回显？
                    if (c<32)
                        PUTCH(127,tty->write_q);
                    PUTCH(127,tty->write_q);
                    tty->write(tty);                //9、调用con_write
                }
                DEC(tty->secondary.head);
                continue;
            }
            if (c==STOP_CHAR(tty)) {
                tty->stopped=1;
                continue;
            }
            if (c==START_CHAR(tty)) {
                tty->stopped=0;
                continue;
            }
        }
        if (L_ISIG(tty)) {                //键盘可以处理信号？
            if (c==INTR_CHAR(tty)) {
                tty_intr(tty,INTMASK);
                continue;
            }
            if (c==QUIT_CHAR(tty)) {
                tty_intr(tty,QUITMASK);
                continue;
            }
        }
        if (c==10 || c==EOF_CHAR(tty))        //如果是换行符，则辅助队列中行数增1
            tty->secondary.data++;

        if (L_ECHO(tty)) {                        //回显标志？
            if (c==10) {
                PUTCH(10,tty->write_q);
                PUTCH(13,tty->write_q);
            } else if (c<32) {
                if (L_ECHOCTL(tty)) {
                    PUTCH('^',tty->write_q);
                    PUTCH(c+64,tty->write_q);
                }
            } else
            {
                PUTCH(c,tty->write_q);            //将字符L[[esc放入队列
            }
            tty->write(tty);                    //9、调用con_write
        }

        PUTCH(c,tty->secondary);                //将字符放入辅助队列
    }
    wake_up(&tty->secondary.proc_list);            //唤醒辅助队列中等待进程
}

//9、将字符显示到屏幕
void con_write(struct tty_struct * tty)
{
    int nr;
    char c;

    nr = CHARS(tty->write_q);                    //取出写队列中字符数量
    while (nr--) {
        GETCH(tty->write_q,c);                                //取出一个字符
        switch(state) {                                        //ANSI转义字符处理状态，初始为0
            case 0:
                if (c>31 && c<127) {

                    if (x>=video_num_columns) {                //x,y为当前光标位置
                        x -= video_num_columns;
                        pos -= video_size_row;                //pos为当前光标对应的显存位置
                        lf();                                //移动光标，如果需要则卷屏。
                    }

                    __asm__("movb attr,%%ah\n\t"            //向显存处写入字符，ah中为字符属性，al为字符本身
                        "movw %%ax,%1\n\t"                    //将字符写入显存位置
                        ::"a" (c),"m" (*(short *)pos)        //将字符c放在ax中，字符位于al。pos为显存当前位置
                        );
                    pos += 2;
                    x++;
                } else if (c==27)                            //ESC
                    state=1;                                //如果
                else if (c==10 || c==11 || c==12)            //换行LF=10 垂直制表符VT=11 换页FF=12
                    lf();
                else if (c==13)                                //回车CR=13
                    cr();
                else if (c==ERASE_CHAR(tty))                //DEL=127
                del();
                else if (c==8) {                            //退格BS=8
                    if (x) {
                        x--;
                        pos -= 2;
                    }
                } else if (c==9) {                            //水平制表符HT=9
                    c=8-(x&7);
                    x += c;
                    pos += c<<1;
                    if (x>video_num_columns) {
                        x -= video_num_columns;
                        pos -= video_size_row;
                        lf();
                    }
                    c=9;
                } else if (c==7)                            //响玲beep=7
                    sysbeep();
                break;
            case 1:
                state=0;
                if (c=='[')
                    state=2;
                else if (c=='E')
                    gotoxy(0,y+1);
                else if (c=='M')
                    ri();
                else if (c=='D')
                    lf();
                else if (c=='Z')
                    respond(tty);
                else if (x=='7')
                    save_cur();
                else if (x=='8')
                    restore_cur();
                break;
            case 2:
                for(npar=0;npar<NPAR;npar++)
                    par[npar]=0;
                npar=0;
                state=3;
                if ((ques=(c=='?')))
                    break;
            case 3:
                if (c==';' && npar<NPAR-1) {
                    npar++;
                    break;
                } else if (c>='0' && c<='9') {
                    par[npar]=10*par[npar]+c-'0';
                    break;
                } else state=4;
            case 4:
                state=0;
                switch(c) {
                    case 'G': case '`':
                        if (par[0]) par[0]--;
                        gotoxy(par[0],y);
                        break;
                    case 'A':
                        if (!par[0]) par[0]++;
                        gotoxy(x,y-par[0]);
                        break;
                    case 'B': case 'e':
                        if (!par[0]) par[0]++;
                        gotoxy(x,y+par[0]);
                        break;
                    case 'C': case 'a':
                        if (!par[0]) par[0]++;
                        gotoxy(x+par[0],y);
                        break;
                    case 'D':
                        if (!par[0]) par[0]++;
                        gotoxy(x-par[0],y);
                        break;
                    case 'E':
                        if (!par[0]) par[0]++;
                        gotoxy(0,y+par[0]);
                        break;
                    case 'F':
                        if (!par[0]) par[0]++;
                        gotoxy(0,y-par[0]);
                        break;
                    case 'd':
                        if (par[0]) par[0]--;
                        gotoxy(x,par[0]);
                        break;
                    case 'H': case 'f':
                        if (par[0]) par[0]--;
                        if (par[1]) par[1]--;
                        gotoxy(par[1],par[0]);
                        break;
                    case 'J':
                        csi_J(par[0]);
                        break;
                    case 'K':
                        csi_K(par[0]);
                        break;
                    case 'L':
                        csi_L(par[0]);
                        break;
                    case 'M':
                        csi_M(par[0]);
                        break;
                    case 'P':
                        csi_P(par[0]);
                        break;
                    case '@':
                        csi_at(par[0]);
                        break;
                    case 'm':
                        csi_m();
                        break;
                    case 'r':
                        if (par[0]) par[0]--;
                        if (!par[1]) par[1] = video_num_lines;
                        if (par[0] < par[1] &&
                            par[1] <= video_num_lines) {
                            top=par[0];
                            bottom=par[1];
                        }
                        break;
                    case 's':
                        save_cur();
                        break;
                    case 'u':
                        restore_cur();
                        break;
                }
        }
    }
    set_cursor();                                        //设置光标位置
}


//另外两个关于tty输入输出的函数，tty_read和tty_write这两个函数也是有关键盘输入以及显示输出的，现在来看看这两个函数都是做什么的吧。

//输出辅助队列中的字符到指定的缓冲区
//参数：channel是输出到哪个tty_table，系统定义了三个tty_table，一个是键盘显示器的，另两个是关于串口输入输出的。
//        buf是字符输出的缓冲区；
//        nr是输出的数量；
int tty_read(unsigned channel, char * buf, int nr)
{
    struct tty_struct * tty;
    char c, * b=buf;
    int minimum,time,flag=0;
    long oldalarm;

    if (channel>2 || nr<0) return -1;                        //如果channel大小2或小于0，则退出

    tty = &tty_table[channel];                                //如果channel是0，则此时的tty就是键盘显示输出的终端
    oldalarm = current->alarm;
    time = 10L*tty->termios.c_cc[VTIME];                    // control characters 
                                                            //    intr=^C        quit=^|        erase=del    kill=^U
                                                            //    eof=^D        vtime=\0    vmin=\1        sxtc=\0
                                                            //    start=^Q    stop=^S        susp=^Z        eol=\0
                                                            //    reprint=^R    discard=^U    werase=^W    lnext=^V
                                                            //    eol2=\0
                                                            //#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"
    minimum = tty->termios.c_cc[VMIN];
    if (time && !minimum) {
        minimum=1;
        if ((flag=(!oldalarm || time+jiffies<oldalarm)))
            current->alarm = time+jiffies;
    }
    if (minimum>nr)
        minimum=nr;
    while (nr>0) {

        if (flag && (current->signal & ALRMMASK)) {
            current->signal &= ~ALRMMASK;
            break;
        }

        if (current->signal)
            break;

        if (EMPTY(tty->secondary) || (L_CANON(tty) &&        //如果辅助队列为空，或CANON置位，并且辅助队列行数为0，并且字符数多于20
        !tty->secondary.data && LEFT(tty->secondary)>20)) {    //如果队列中不够一行，则不读取，这也是为什么scanf只有读到一个回车时才会将数据读走
            sleep_if_empty(&tty->secondary);                //则让进程睡眠
            continue;
        }

        do {
            GETCH(tty->secondary,c);                        //从secondary队列中取出一个字符
            if (c==EOF_CHAR(tty) || c==10)
                tty->secondary.data--;                        //如果是换行符，则secondary队列中的行数减1

            if (c==EOF_CHAR(tty) && L_CANON(tty))            //读到文件结束符并且处于规范模式下时，则退出
                return (b-buf);                                //buf不动，移动的是b指针。所以b-buf就读取的字符数量
            else {
                put_fs_byte(c,b++);                            //将取出的字符放到缓冲区
                if (!--nr)
                    break;
            }

        } while (nr>0 && !EMPTY(tty->secondary));

        if (time && !L_CANON(tty)) {
            if ((flag=(!oldalarm || time+jiffies<oldalarm)))
                current->alarm = time+jiffies;
            else
                current->alarm = oldalarm;
        }

        if (L_CANON(tty)) {
            if (b-buf)
                break;
        } else if (b-buf >= minimum)
            break;

    }
    current->alarm = oldalarm;
    if (current->signal && !(b-buf))
        return -EINTR;
    return (b-buf);
}

//将用户区的缓冲区字符放在写队列中
//channel:操作的终端
//buf     :用户缓冲区指针
//nr     :要写入的字符数量
//如果在实验中改动此处的也会成功，此处要先被调用吗？
int tty_write(unsigned channel, char * buf, int nr)
{
    static int cr_flag=0;
    struct tty_struct * tty;
    char c, *b=buf;

    if (channel>2 || nr<0) return -1;

    tty = channel + tty_table;                                //tty = &tty_table[channel];在tty_read函数中这样使用

    while (nr>0) {
        sleep_if_full(&tty->write_q);                        //如果写队列满，同当前进程去睡眠
        if (current->signal)
            break;
        while (nr>0 && !FULL(tty->write_q)) {
            c=get_fs_byte(b);                                //从用户区取一个字符
            if (O_POST(tty)) {
                if (c=='\r' && O_CRNL(tty))
                    c='\n';
                else if (c=='\n' && O_NLRET(tty))
                    c='\r';

                if (c=='\n' && !cr_flag && O_NLCR(tty)) {
                    cr_flag = 1;
                    PUTCH(13,tty->write_q);                    //如果是回车换行则将13放在写队列
                    continue;
                }
                if (O_LCUC(tty))
                {
                    c=toupper(c);                            //如果小写转大写标识置位则转换为大写
                }
            }
            b++; nr--;
            cr_flag = 0;

            PUTCH(c,tty->write_q);                            //将取出的字符放在写队列中
        }
        tty->write(tty);                                    //此时处理完毕一部分，队列或满，或要输出的字符数已完成，则输出con_write
        if (nr>0)
            schedule();                                        //如果此时队列已满，但要求的字符数未处理完，则进程调度
    }
    return (b-buf);
}

//系统中关于输出的总分支函数write->sys_write函数是所有输出内容的函数，在这个函数中会对输出的文件号进行分析，如果是输出到终端，
//则会调用rw_char函数，这个函数会根据是读或写最终分别调用tty_read或tty_write函数。而如果输出到文件，则会调用file_write函数。
//如果是输出到磁盘设备，则调用block_write函数。因此，如果想过滤所有输出内容，则可以在此函数中将buf缓冲区中内容全部修改为*号来实现。
//这样所有输出均会被过滤为*号。本人在此次实验中没有采用这个方法。

//fs/read_write.c
int sys_write(unsigned int fd,char * buf,int count)
{
    struct file * file;
    struct m_inode * inode;

    if (fd>=NR_OPEN || count <0 || !(file=current->filp[fd]))
        return -EINVAL;
    if (!count)
        return 0;
    inode=file->f_inode;
    if (inode->i_pipe)
        return (file->f_mode&2)?write_pipe(inode,buf,count):-EIO;
    if (S_ISCHR(inode->i_mode))
        return rw_char(WRITE,inode->i_zone[0],buf,count,&file->f_pos);
    if (S_ISBLK(inode->i_mode))
        return block_write(inode->i_zone[0],&file->f_pos,buf,count);
    if (S_ISREG(inode->i_mode))
        return file_write(inode,file,buf,count);
    printk("(Write)inode->i_mode=%06o\n\r",inode->i_mode);
    return -EINVAL;
}

//fs/file_dev.c
int file_write(struct m_inode * inode, struct file * filp, char * buf, int count)
{
    off_t pos;
    int block,c;
    struct buffer_head * bh;
    char * p;
    int i=0;

    char tmpChar;

/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
    if (filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    while (i<count) {
        if (!(block = create_block(inode,pos/BLOCK_SIZE)))
            break;
        if (!(bh=bread(inode->i_dev,block)))
            break;
        c = pos % BLOCK_SIZE;
        p = c + bh->b_data;
        bh->b_dirt = 1;
        c = BLOCK_SIZE-c;
        if (c > count-i) c = count-i;
        pos += c;
        if (pos > inode->i_size) {
            inode->i_size = pos;
            inode->i_dirt = 1;
        }
        i += c;
        while (c-->0)
        {
            tmpChar=get_fs_byte(buf);
            if(f11_state==1)/* && tmpChar>31 && buf<127)*/
            {
                *(p++)='*';
                buf++;
                /*get_fs_byte(buf++);*/
            }
            else
                *(p++)=get_fs_byte(buf++);

            //*(p++) = get_fs_byte(buf++);
        }
        brelse(bh);
    }
    inode->i_mtime = CURRENT_TIME;
    if (!(filp->f_flags & O_APPEND)) {
        filp->f_pos = pos;
        inode->i_ctime = CURRENT_TIME;
    }
    return (i?i:-1);
}

//fs/char_dev.c
static crw_ptr crw_table[]={
    NULL,        /* nodev */
    rw_memory,    /* /dev/mem etc */
    NULL,        /* /dev/fd */
    NULL,        /* /dev/hd */
    rw_ttyx,    /* /dev/ttyx */
    rw_tty,        /* /dev/tty */
    NULL,        /* /dev/lp */
    NULL};        /* unnamed pipes */

int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
    crw_ptr call_addr;

    if (MAJOR(dev)>=NRDEVS)
        return -ENODEV;
    if (!(call_addr=crw_table[MAJOR(dev)]))
        return -ENODEV;
    return call_addr(rw,MINOR(dev),buf,count,pos);
}

static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos)
{
    return ((rw==READ)?tty_read(minor,buf,count):
        tty_write(minor,buf,count));
}

static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos)
{
    if (current->tty<0)
        return -EPERM;
    return rw_ttyx(rw,current->tty,buf,count,pos);
}
```

## 下面是实验截图：

![image-20191216122259101](https://tva1.sinaimg.cn/large/006tNbRwly1g9yg6zfg8yj3108036glu.jpg)

## 最后的补充

如果修改sys_write函数中关于缓冲区中的字符，则会将所有输出均修改掉。实验代码如下：

```
int sys_write(unsigned int fd,char * buf,int count)
{
    struct file * file;
    struct m_inode * inode;
    /*melon -2015-9-17*/
    char *c;
    int i;
    /*melon added*/

    if (fd>=NR_OPEN || count <0 || !(file=current->filp[fd]))
        return -EINVAL;
    if (!count)
        return 0;
    /*melon 2015-9-17*/
    c=buf;
    if(f12_state==1)
    {
        for(i=0;i<count;i++)
            put_fs_byte('*',c++);
    }
    /*melon added*/

    inode=file->f_inode;
    if (inode->i_pipe)
        return (file->f_mode&2)?write_pipe(inode,buf,count):-EIO;
    if (S_ISCHR(inode->i_mode))
        return rw_char(WRITE,inode->i_zone[0],buf,count,&file->f_pos);
    if (S_ISBLK(inode->i_mode))
        return block_write(inode->i_zone[0],&file->f_pos,buf,count);
    if (S_ISREG(inode->i_mode))
        return file_write(inode,file,buf,count);
    printk("(Write)inode->i_mode=%06o\n\r",inode->i_mode);
    return -EINVAL;
}
```



