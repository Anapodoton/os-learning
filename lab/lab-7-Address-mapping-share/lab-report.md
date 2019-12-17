

1. **跟踪地址翻译过程**：

test.c

```
#include <stdio.h>

int i = 0x12345678;
int main(void)
{
    printf("The logical/virtual address of i is 0x%08x", &i);
    fflush(stdout);
    while (i)
        ;
    return 0;
}
```

sudo ./mount-hdc

cp test.c hc/usr/root



通过地址ds:offset 查找目标物理地址的步骤列如下：

- 通过gdtr寄存器（存放物理地址）+ ldtr（存放选择子），来获得进程ldt的物理地址。
- 再根据段选择器（DS/CS）定位，段选择符在LDT中的位置，用以获得线性地址的基地址，其中LDT和GDT中的选择符结构一致。
- 得到的线性基地址+段内偏移量即为线性地址。
- 根据线性地址利用页表映射成物理地址，首先根据CR3寄存器获得页表项目录（page dir）基地址 .
- 根据线性地址的前10bit获取page table 在page dir中的位置。获取page table的物理地址，注意对于该物理的第三位需要ignor。
- 再根据线性地址的12-21bit来获取物理页项在页表（page table）中的位置。 获取物理页后，再根据线性地址中的低12位获取，目标数据。寻址完成。



- 最重要的四步：
  1. 通过段寄存器确定查询的是LDT表或者是GDT表
  2. 结合ldtr和ldtr寄存器来获得线性地址
  3. 根据CR3中的目录基址获得线性地址对应的物理地址
- 在执行一次test会有的异同：
  1. 段基址可能会变化，因为段基址是根据进程次序由操作系统分配的64M空间。
  2. 数据段内偏移量不会偏移量[逻辑地址]不会变，因为这是编译后就不变了。



**编写多进程的生产者—消费者程序**

用共享内存做缓冲区.

1. mm/shm.c

   shm.c 增加2个系统调用函数，分别用于分配物理页空间，和绑定用户线性地址空间。

   ```c
   // int shmget(char * name);
   // void * shmat(int shmid);
   #include <asm/segment.h>
   #include <linux/kernel.h>
   #include <unistd.h>
   #include <string.h>
   #include <linux/sched.h>
   
   #define SHM_COUNT 20
   #define SHM_NAME_SIZE 20
   
   struct struct_shm_tables
   {
   	int occupied;
   	char name[SHM_NAME_SIZE];
   	long addr;
   } shm_tables[SHM_COUNT];
   
   int find_shm_location(char *name)
   {
   	int i;
   	for(i=0; i<SHM_COUNT; i++)
   	{
   		if(!strcmp(name,shm_tables[i].name) && shm_tables[i].occupied ==1)
   		{
   			return i;
   		}
   	}
   	return -1;
   }
   /*shmget()会新建/打开一页内存，并返回该页共享内存的shmid*/
   int sys_shmget(char * name)
   {
   	int i,shmid;
   	char tmp[SHM_NAME_SIZE];
   	for(i=0; i<SHM_NAME_SIZE; i++)
   	{
   		tmp[i] = get_fs_byte(name+i);
   		if(tmp[i] == '\0') break;
   	}
   	shmid = find_shm_location(tmp);
   	if( shmid != -1) 
   	{
   		return shmid;
   	}
   	for(i=0; i<SHM_COUNT; i++)
   	{
   		if(shm_tables[i].occupied == 0)
   		{
   			strcpy(shm_tables[i].name,tmp);
   			shm_tables[i].occupied = 1;
   			shm_tables[i].addr = get_free_page();
   			return i;
   		}
   	}
   	printk("SHM Number limited!\n");
   	return -1;
   }
   
   /*shmat()会将shmid指定的共享页面映射到当前进程的虚拟地址空间中，并将其首地址返回*/
   void * sys_shmat(int shmid)
   {
   	if(shm_tables[shmid].occupied != 1)
   	{
   		printk("SHM not exists!\n");
   		return -1;
   	}
   	put_page(shm_tables[shmid].addr,current->brk + current->start_code);
   	return (void*)current->brk;
   }
   ```

2. mm/memory.c

   增加put_shm_page 用于将共享内存页增加到线性地址空间内.

   

3. kernel/sem.c

   ```c
   #define __LIBRARY__  
   #include <unistd.h>  
   #include <linux/sched.h>  
   #include <linux/kernel.h>  
   #include <asm/segment.h>  
   #include <asm/system.h>   
   
   #define SEM_COUNT 32 
   sem_t semaphores[SEM_COUNT]; 
   /*队列相关操作，rear始终是下一个待写入的位置，front始终是队列第一个元素*/
   void init_queue(sem_queue* q)  
   {  
       q->front = q->rear = 0; 
   }
   
   int is_empty(sem_queue* q)
   {
   	return q->front == q->rear?1:0;
   }
   /*留下标QUE_LEN-1不用，判断是否慢*/
   int is_full(sem_queue* q)
   {    
   	return (q->rear+1)%QUE_LEN == q->front?1:0;
   }
   /*获得队列头第一个任务*/
   struct task_struct * get_task(sem_queue* q)
   {
   	if(is_empty(q)) 
   	{
   		printk("Queue is empty!\n");
   		return NULL;
   	}
   	struct task_struct *tmp = q->wait_tasks[q->front]; 
   	q->front = (q->front+1)%QUE_LEN;
   	return tmp;
   }
   /*任务插入队列尾*/
   int insert_task(struct task_struct *p,sem_queue* q)
   {
   	// printk("Insert %d",p->pid);
   	if(is_full(q))
   	{
   		printk("Queue is full!\n");
   		return -1;
   	}
   	q->wait_tasks[q->rear] = p;
   	q->rear = (q->rear+1)%QUE_LEN;
   	return 1;
   }
   /*信号量是否已打开，是返回位置*/
   int sem_location(const char* name)
   {  
       int i;
       for(i = 0;i < SEM_COUNT; i++)  
       {  
           if(strcmp(name,semaphores[i].name) == 0 && semaphores[i].occupied == 1) 
           {     
               return i;  
           }  
       }  
       return -1; 
   }  
   /*打开信号量*/
   sem_t* sys_sem_open(const char* name,unsigned int value)
   {
   	char tmp[16];
   	char c;
   	int i;
   	for( i = 0; i<16; i++)
   	{
   		c = get_fs_byte(name+i);
   		tmp[i] = c;
   		if(c =='\0') break;
   	}
   	if(c >= 16) 
   	{ 	
   		printk("Semaphore name is too long!");
   		return NULL;
   	}
   	if((i = sem_location(tmp)) != -1)
   	{
   		return &semaphores[i];
   	}
   	for(i = 0;i< SEM_COUNT; i++)
   	{
   		if(!semaphores[i].occupied)
   		{
   			strcpy(semaphores[i].name,tmp);
   			semaphores[i].occupied = 1;
   			semaphores[i].value = value;
   			init_queue(&(semaphores[i].wait_queue));
   			// printk("%d %d %d %s\n",semaphores[i].occupied,i,semaphores[i].value,semaphores[i].name);
   			// printk("%p\n",&semaphores[i]); 
   			return &semaphores[i];
   		}
   	}	
   	printk("Numbers of semaphores are limited!\n");
   	return NULL;
   }  
   /*P原子操作*/
   int sys_sem_wait(sem_t* sem)
   {
   	cli();
   	sem->value--;
   	if(sem->value < 0)
   	{
   		/*参见sleep_on*/
   		current->state = TASK_UNINTERRUPTIBLE;
   		insert_task(current,&(sem->wait_queue));
   		schedule();
   	}
   	sti();
   	return 0;
   }
   /*V原子操作*/
   int sys_sem_post(sem_t* sem)
   {
   	cli();
   	struct task_struct *p;
   	sem->value++;
   	if(sem->value <= 0)
   	{
   		p = get_task(&(sem->wait_queue));
   		if(p != NULL)
   		{
   			(*p).state = TASK_RUNNING;
   		}
   	}
   	sti();
   	return 0;
   }
   /*释放信号量*/
   int sys_sem_unlink(const char *name)  
   {  
       char tmp[16];
       char c;
   	int i;
   	for( i = 0; i<16; i++)
   	{
   		c = get_fs_byte(name+i);
   		tmp[i] = c;
   		if(c =='\0') break;
   	}
   	if(c >= 16) 
   	{
   		printk("Semphore name is too long!");
   		return -1;
   	}
       int ret = sem_location(tmp); 
       if(ret != -1)
       {
       	semaphores[ret].value = 0;
       	strcpy(semaphores[ret].name,"\0");
       	semaphores[ret].occupied = 0;
       	return 0;
       }   
       return -1;  
   }  
   ```

   

4. 修改unistd.h
   添加我们新增的系统调用的编号。
   添加的代码如下：

   ```
   #define __NR_sem_open 	72
   #define __NR_sem_wait	73
   #define __NR_sem_post	74
   #define __NR_sem_unlink 75
   #define __NR_shmget 76
   #define __NR_shmat 77
   ```

    

5. 修改system_call.s
   由于新增了4个系统调用，所以需要修改总的系统调用的和值。
   修改代码如下：

   ```
   1 nr_system_calls = 78    /* !!! */
   ```

    

6. 修改sys.h
   要在linux-0.11/include/linux/sys.h中，声明这4个新增的函数。
   修改代码如下：

   ```
   extern int sys_sem_open();
   extern int sys_sem_wait();
   extern int sys_sem_post();
   extern int sys_sem_unlink();
   extern int sys_shmget();
   extern void* sys_shmat();
   
   fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
   sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
   sys_unlink, sys_execve, sys_chdir, sys_time, sys_mknod, sys_chmod,
   sys_chown, sys_break, sys_stat, sys_lseek, sys_getpid, sys_mount,
   sys_umount, sys_setuid, sys_getuid, sys_stime, sys_ptrace, sys_alarm,
   sys_fstat, sys_pause, sys_utime, sys_stty, sys_gtty, sys_access,
   sys_nice, sys_ftime, sys_sync, sys_kill, sys_rename, sys_mkdir,
   sys_rmdir, sys_dup, sys_pipe, sys_times, sys_prof, sys_brk, sys_setgid,
   sys_getgid, sys_signal, sys_geteuid, sys_getegid, sys_acct, sys_phys,
   sys_lock, sys_ioctl, sys_fcntl, sys_mpx, sys_setpgid, sys_ulimit,
   sys_uname, sys_umask, sys_chroot, sys_ustat, sys_dup2, sys_getppid,
   sys_getpgrp, sys_setsid, sys_sigaction, sys_sgetmask, sys_ssetmask,
   sys_setreuid, sys_setregid, sys_sem_open, sys_sem_wait, sys_sem_post,
   sys_sem_unlink, sys_shmget, sys_shmat };
   ```

7. 修改linux-0.11/kernel目录下的Makefile
   修改代码如下：

   ```
   ......
   OBJS  = sched.o system_call.o traps.o asm.o fork.o \
   panic.o printk.o vsprintf.o sys.o exit.o \
   signal.o mktime.o sem.o shm.o
   ......
   
   ### Dependencies:
   sem.s sem.o: sem.c ../include/unistd.h ../include/asm/segment.h \
   ../include/asm/system.h ../include/linux/kernel.h ../include/linux/sched.h
   shm.s shm.o: shm.c ../include/unistd.h ../include/linux/kernel.h \
   ../include/linux/sched.h ../include/linux/mm.h ../include/error.h
   ......
   ```


Answers:

1. 对于地址映射实验部分，列出你认为最重要的那几步（不超过 4 步），并给出你获得的实验数据。

前提：知道段选择子（16位），段描述符（64位），页目录表及页表项（32位）的意义
Step1:首先需要获得逻辑地址，LDT的地址：
  逻辑地址 0x00003004  
  全局描述符表物理地址：  gdtr:base=0x00005cb8, limit=0x7ff
  而局部描述符选择子：ldtr:s=0x0068 二进制0000,0000,0110,1000b 1101即GDT表第14项
     bochs:4> xp /2w 0x5cb8 + 13*8        
[bochs]:
0x00005d20 <bogus+       0>:    0x52d40068      0x000082fd
  即LDT的物理地址： 0x00fd52d4     

Step2:然后需要获得线性地址：
  DS段选择子 ds:s=0x0017 即LDT表中第3项
<bochs:5> xp /2w 0xfd52d4 + 2*8
[bochs]:
0x00fd52e4 <bogus+       0>:    0x00003fff      0x10c0f300
  即DS段基地址为 0x10000000
  所以线性地址为：0x10003004

Step3:其次获得页目录及页表地址计算得物理地址
  CR3内容: CR3=0x00000000
  虚拟地址对应页目录第65项，页表第4项，页表内偏移0x004
  查询页目录第65项：
<bochs:6> xp /w 64*4
[bochs]:
0x00000100 <bogus+       0>:    0x00fa5027
  页表物理地址为： 0x00fa5000, 查询第4项
<bochs:7> xp /w 0xfa5000 + 3*4
[bochs]:
0x00fa500c <bogus+       0>:    0x00fa3067
  页框物理地址：0x00fa3000 加上偏移0x004
  计算得到物理地址： 0x00fa3004
<bochs:8> xp /w 0x00fa3004
[bochs]:
0x00fa3004 <bogus+       0>:    0x12345678



2. test.c 退出后，如果马上再运行一次，并再进行地址跟踪，你发现有哪些异同？为什么？

逻辑地址和虚拟地址不变，页目录地址是操作系统放置的， 物理分页变了，所以物理地址也变了。原因是每次进程加载后都有64M的虚拟地址空间，而且，逻辑地址没有变化。操作系统加载程序时，由于虚拟地址是按nr分配64M，两次运行nr一致，所以虚拟地址没变。

