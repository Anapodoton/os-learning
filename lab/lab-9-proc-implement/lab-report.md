## 在include/sys/stat.h中

新增定义两个宏,24行，36行

```
#define S_IFPROC 0030000
#define S_ISPROC(m) (((m) & S_IFMT) == S_IFPROC)
```

## fs/namei.c文件中

在sys_mknod函数里增加支持新的文件类型，444行

```
if (S_ISBLK(mode) || S_ISCHR(mode) || S_ISPROC(mode))
		inode->i_zone[0] = dev;
```

## 在init/main.c中

增加创建/proc目录和目录下的结点的代码，增加有关的系统调用 

54行：

```
_syscall2(int,mkdir,const char*,name,mode_t,mode)
_syscall3(int,mknod,const char*,filename,mode_t,mode,dev_t,dev)
```

177行：

```
mkdir("/proc",0755);
mknod("/proc/psinfo",S_IFPROC|0444,0);
mknod("/proc/hdinfo",S_IFPROC|0444,1);
mknod("/proc/inodeinfo",S_IFPROC|0444,2);
```

## 在fs/read_write.c中

增加对proc文件进行处理的分支，调用proc_read函数 ,68行

```
if (S_ISPROC(inode->i_mode))
		return proc_read(inode->i_zone[0],&file->f_pos,buf,count);
```

![实验楼](https://dn-simplecloud.shiyanlou.com/5633351518359244962-wm) 

## 在fs/proc.c中

实现proc_read的函数

```
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <stdarg.h>
#include <stddef.h>

extern int vsprintf(char * buf, const char * fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...){
    va_list args; int i;
    va_start(args, fmt);
    i=vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}

int proc_read(int dev, unsigned long * pos, char * buf, int count){
    struct task_struct ** p;
    int output_count=0;
    char * proc_buf=NULL;
    int file_size=0;
    int offset=*pos;

    struct super_block * sb; 
    struct buffer_head * bh;
    int total_blocks, total_inodes;
    int used_blocks=0, free_blocks=0;
    int i,j,k;
    char * db=NULL;

    sb=get_super(current->root->i_dev);
    total_blocks = sb->s_nzones;
    total_inodes=sb->s_ninodes;

    if(dev==0){  /*psinfo*/
        proc_buf=(char *)malloc(sizeof(char *)*1024);
        file_size=sprintf(proc_buf,"pid\tstate\tfather\tcounter\tstart_time\n");

        for(p = &LAST_TASK ; p >= &FIRST_TASK ; --p)
            if(*p)
                file_size+=sprintf(proc_buf+file_size,"%d\t%d\t%d\t%d\t%d\n",(*p)->pid,(*p)->state,(*p)->father,(*p)->counter,(*p)->start_time);

        *(proc_buf+file_size)='\0';
    }

    if(dev==1) {  /*hdinfo*/
        for(i=0;i<sb->s_zmap_blocks;i++){
            bh=sb->s_zmap[i];
            db=(char*)bh->b_data;
            for(j=0;j<1024;j++){
                for(k=1;k<=8;k++){
                        if((used_blocks+free_blocks)>=total_blocks)
                            break;
                        if( *(db+j) & k)
                            used_blocks++;
                        else
                            free_blocks++;
                }
            }
        }

        proc_buf=(char*)malloc(sizeof(char*)*512);
        file_size=sprintf(proc_buf,"s_imap_blocks:%d\ns_zmap_blocks:%d\n",sb->s_imap_blocks,sb->s_zmap_blocks);
        file_size+=sprintf(proc_buf+file_size,"total_blocks:%d\nfree_blcoks:%d\nused_blocks:%d\ntotal_indoes:%d\n",total_blocks,free_blocks,used_blocks,total_inodes);
    }

    while(count>0){  /*put the data into buffer*/
        if(offset>file_size)
            break;
        put_fs_byte(*(proc_buf+offset),buf++);
        offset++;
        output_count++;
        count--;
    }

    (*pos)+=output_count;  /*change the file->f_pos*/

    free(proc_buf);
    return output_count;
}
```

## 修改fs/Makefile



![实验楼](https://dn-simplecloud.shiyanlou.com/5633351518359722190-wm) ![实验楼](https://dn-simplecloud.shiyanlou.com/5633351518359728322-wm)

```
proc_dev.o:proc_dev.c ../include/errno.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/sys/types.h \
  ../include/linux/mm.h ../include/signal.h ../include/linux/kernel.h \
  ../include/asm/segment.h ../include/asm/system.h ../include/stdarg.h
```

 

## 运行系统

分别用cat命令查看psinfo和hdinfo 

![](https://i.loli.net/2019/12/17/xG83bIERnt2zSo1.png)

## 实验问题

#### 1. 如果要求你在psinfo之外再实现另一个结点，具体内容自选，那么你会实现一个给出什么信息的结点？为什么？

内存信息，因为cpu直接对内存进行操作，是多进程图像的重要组成部分，了解内存的占用情况可以更加清晰的明白操作系统对硬件的管理状况。meminfo，可以获得内存相关信息，看那些程序占用内存较多，方便管理。

#### 2. 一次read()未必能读出所有的数据，需要继续read()，直到把数据读空为止。而数次read()之间，进程的状态可能会发生变化。你认为后几次read()传给用户的数据，应该是变化后的，还是变化前的？ 如果是变化后的，那么用户得到的数据衔接部分是否会有混乱？如何防止混乱？ 如果是变化前的，那么该在什么样的情况下更新psinfo的内容？

应该保持数据的完整性，传递变化前的数据。在read的过程中如果发生状态变化，可以先让改写的进程先睡眠，等读完后重置指针于0位置处，再唤醒进程更新数据。

是变化前的，在读取位置f_pos为0时才更新psinfo内容。该inode对应的i_zone[0]依然存在。也就是说，只是从inode映射中取消映射该inode，但是实际上硬盘上的数据还在。