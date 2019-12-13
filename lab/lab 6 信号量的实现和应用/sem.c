#define __LIBRARY__  
#include <unistd.h>  
#include <linux/sched.h>  
#include <linux/kernel.h>  
#include <asm/segment.h>  
#include <asm/system.h>   

#define SEM_COUNT 32 
/* sem_t 是信号量类型，根据实现的需要自定义 */
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

/*sem_open() 的功能是创建一个信号量，或打开一个已经存在的信号量。*/
/*name 是信号量的名字。不同的进程可以通过提供同样的 name 而共享同一个信号量。
如果该信号量不存在，就创建新的名为 name 的信号量；如果存在，就打开已经存在的名为 name 的信号量。*/
/*value 是信号量的初值，仅当新建信号量时，此参数才有效，其余情况下它被忽略。
当成功时，返回值是该信号量的唯一标识（比如，在内核的地址、ID 等），由另两个系统调用使用。如失败，返回值是 NULL。*/
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
/*如果继续运行的条件不满足，则令调用进程等待在信号量 sem 上。返回 0 表示成功，返回 -1 表示失败。*/
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
/*sem_post() 就是信号量的 V 原子操作。如果有等待 sem 的进程，它会唤醒其中的一个。返回 0 表示成功，返回 -1 表示失败。*/
int sys_sem_post(sem_t* sem)
{
	cli();//关闭时钟中断
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
	sti();//打开时钟中断
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