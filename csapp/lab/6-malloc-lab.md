[Malloc Lab - 实现一个动态内存分配](http://wdxtub.com/csapp/thick-csapp-lab-6/2016/04/16/)

这次我们会实现自己的 malloc, free, realloc, calloc 函数，并借此深入理解堆中的内存分配机制。更有意思的是，gdb 在这次实验中基本没太多用处，我们要自己写堆检查器用来 debug。

<!--ts-->
      * [任务目标](#任务目标)
      * [提示与测试](#提示与测试)
      * [解题思路](#解题思路)
      * [基础知识复习](#基础知识复习)
         * [宏与内联函数](#宏与内联函数)
         * [指针复习](#指针复习)
         * [Malloc](#malloc)
      * [总结](#总结)

<!-- Added by: anapodoton, at: Thu Feb  6 23:01:02 CST 2020 -->

<!--te-->

## 任务目标

实现自己的 `malloc`, `free`, `realloc`, `calloc` 函数。

我们需要做的是完成在 `mm.c` 中的以下几个函数

```
int mm_init(void);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);
void mm_checkheap(int);
```

复制

`mm-naive.c` 中有一个简单的实现，另外 `mm-textbook.c` 中实现了课本中提到的 implicit list allocator。具体的函数介绍如下：

- `mm-init`：在这里执行所有的初始化操作，包括分配初始的堆区域。注意，必须在这里重新初始化所有的全局变量，并且不要调用 `mem.init` 函数。成功的话返回 0 ，否则返回 -1

- `malloc`：至少需要分配 `size` 这么大的空间（可能因为对齐的原因会更大一点，8 byte 对齐），不能超出堆的范围，也不能覆盖其他已分配的区域

- `free`：释放 `ptr` 指针指向的区域（这个区域必须是已分配的），`free(NULL)` 什么都不做

- ```
  realloc
  ```

  ：重新分配，根据传入指针的不同，有不同的表现

  - `ptr` 为 NULL 时，等同于 `malloc(size)`
  - `size` 为 0 时，等同于 `free(ptr)`，需要返回 NULL
  - `ptr` 不为 NULL 时，一定是指向一个已分配的空间的，就根据新的 size 的大小进行调整，并让 `ptr` 指向新的地址（如果是新地址的话），并且旧的区域应该被释放。另外需要注意的是，需要把原来 block 的值复制过去

- `calloc`：分配一个有 `nmemb` 个大小为 `size` 的数组，这个函数不评分，只要简单实现即可

- `mm_checkheap`：扫描堆并检查其状态，注意，只有在检测到错误时才输出内容并调用 `exit` 退出。`mm_heapchecker(__Line__);` 传入的参数是当前行数，方便大家找到错误位置。

`memlib.c` 模拟了内存系统，可以调用下面的方法来得到响应的信息：

- `void *mem_sbrk(int incr)`：让堆扩展 `incr` 个字节，并返回新分配的地址的头指针
- `void *mem_heap_lo(void)`：返回指向堆的第一个字节的指针
- `void *mem_heap_hi(void)`：返回指向堆的最后一个字节的指针
- `size_t mem_heapsize(void)`：返回当前的堆大小
- `size_t mem_pagesize(void)`：返回系统的 page size

head checker 需要做的工作有：

- 检查堆(implicit list, explicit list, segregated list)
  - Check epilogue and prologue blocks
  - Check each block’s address alignment
  - Check heap boundaries
  - Check each block’s header and footer: size(minimum size, slignment), previous/net allocate/free bit consistency, header and footer matching each other
  - Check coalescing: no two consecutive free blocks in the heap
- 检查 free list(explicit list, segregated list)
  - All next/previous pointer are consistent (is A’s next pointer points ot B, B’s previous pointer should point to A)
  - All free list pointers points between `mem_heap_lo()` and `mem_heap_hi()`
  - Count free blocks by iterating through every block and traversing free list by pointers and see if they match
  - All blocks in each list bucket fall within bucket size range(segregated list)

需要注意的地方：

- 不能改动 `mm.h`，但是可以在 `mm.c` 中添加 `static` 方法使代码更好理解
- `mm.c` 中不能定义任何全局 array, tree 或 list，但是可以定义全局 struct 和诸如 integer, float 和 pointer 等变量
- 返回的指针必须是 8-byte 对齐的
- 编译代码不能有警告
- 建议先实现 explcit free list

## 提示与测试

- 因为是 64 位机器，所以指针的大小是 8 字节
- `sizeof(size_t) == 8`
- `gprof` 工具可能会很有用
- 三种组织 free block 的方法：implicit free list, explicit free list, segregated free list
- 三种扫描 free block 的方法：first fit/next fit, blocks sorted by address with first fit, best fit
- 可以随机用上面的方法排列组合
- 可以根据 implicit free list（在 `mm-textbook.c` 中）来实现 explicit free list，然后实现 segregated list

可以通过 `mdriver.c` 来进行测试，每个测试会跑 12 次，一次检测正确性，一次检测空间使用，十次测试性能，下面是具体的参数：

- `-p`：完整测试
- `-t `：在自定义的文件夹中搜索测试文件
- `-f `：进行一个特定的测试
- `-c `：执行特定的测试 1 次，用来检测正确性很方便
- `-h`：输出命令行参数
- `-l`：用真实的 `malloc` 函数来测试，可以比较自己写的代码和系统代码的差距
- `-V`：输出各种信息
- `-v `：设置需要输出的日志等级
- `-d `： 有 0,1,2 三个层级，检查的标准越来越严格
- `-D`：等于 `-d2`
- `-s `：超过 s 秒则认为是超时，默认是永远不会超时的

主要的考察目标是空间使用率以及吞吐量（每秒钟执行的操作数目）

## 解题思路

题目中给出了一个提示，说堆的大小不会超过 $2^32$ 字节，这个是什么意思呢？其实很简单，我们知道 64 位机器中指针的长度是 8 个字节（八八六十四），但是因为堆的大小是有上限的，理论上来说，只要 4 个字节就可以完成整个空间的寻址（只需要记录偏移量即可），这样一来，用来保存结构信息的部分只需要原来的一半，内存的有效使用率自然就上去了。（不过具体作业的时候因为我主要参考 `mm-textbook.c` 中的代码，就没有另外做 4 字节寻址的版本了）

如果有仔细看习题课视频的话，其实助教已经讲得非常清楚了，我的建议如下：

- `mm-naive.c` 很短，一下就可以看完，用来热身
- `mm-textbook.c` 基本上包括各种所需的宏以及存储结构，一定要好好理解清楚
- 确定了具体保存信息的数据结构之后，一定要先完成 heap checker
- 遇到段错误的时候就需要利用 heap checker 找到具体问题所在了
- 最好画简单的示意图，对照着来编码，不然很容易乱
- 很多时候需要用到位操作，确保每个基本操作都没有错（如果用 `mm-textbook.c` 中的就不用担心这个）
- 列表的粒度越细，利用率就越高，但是最好的优化还是从数据结构入手

最后要说的是不用自定义的数据结构应该很难做到 100 分，但是 90 分是没有问题的。

## 基础知识复习

### 宏与内联函数

宏实际上是一个简单的『查找并替换』的过程，会在编译前完成。一般用来定义常量以及简单的操作（这里很容易出错，下面会说）。

定义常量比较简单：`#define NUM_ENTRIES 100` 即可。

定义简单操作则需要注意，比如 `#define twice(x) 2*x` 这样写就是会出问题的，如果在代码中调用 `twice(x+1)` 期望得到的结果应该是 `2x+2`，但是因为宏只是简单的替换，所以会变成 `2x+1`。解决办法是一定要用括号包裹住会被替换的值，比如之前的例子应该改写成 `#define twice(x) (2*(x))`。

使用宏，可以避免函数调用，也就少了很多跳转和对应的栈处理。对于 `malloc` 来说，可以通过宏快速访问 header 信息（例如 payload size, valid）

宏的缺点也同样明显，能够执行的操作不如函数那么强大，并且不会进行拼写和类型检查，很容易出错，出错了也不容易找到问题所在。

内联函数会在编译的时候被写入到代码中，同样因为不需要真实的函数调用，所以效率也很高。一般来说，比较小的函数都可以设置为内联。

两者之间的差别在于：

- 宏在编译前处理
- 内联函数在编译时处理（带有类型检查）
- 宏不能有返回值
- 宏可能会带来一些副作用
- 很难调试宏

具体到这次作业：

- 两个都需要使用
- 宏适合做小的工作，比方说让代码更加容易理解
- 如果宏没有办法完成，先考虑使用内联函数

### 指针复习

指针恐怕是 C 语言最难的一部分了，这里会尽量解释得清楚一些。

先来看看类型转换可能带来的问题：

- 从

   

  ```
  <type_a>*
  ```

   

  转换到

   

  ```
  <type_b>*
  ```

  （一个类型的指针转换为另一个类型的指针）

  - 值并不会改变
  - 改变的是解析引用的行为，比方说原来是 int 指针，那么一次会读 4 个字节，现在转换成了 char 指针，一次就读 1 个字节（这里具体的字节数看是 32 位还是 64 位系统）

- 从

   

  ```
  <type_a>*
  ```

   

  转换到 integer / unsigned int

  - 指针的值实际上就是 8 字节的数字
  - 这是一个很值得利用的特性！
  - 不过也很容易出错就是了

- 从 integer / unsigned int 转换到

   

  ```
  <type_a>*
  ```

  - 这种情况基本不会使用，因为没人知道转换后的指针会指向什么地方

对于指针进行算术运算时，一定要注意跟指针本身的类型是有关的，比方说

```
type_a* pointer = ...;
(void *) pointer1 = (void *)(pointer + a);
```

复制

实际上进行的运算是：

```
pointer1 = pointer + (a * sizeof(type_a))
```

复制

对应的汇编代码是：

```
lea (pointer, a, sizeof(type_a)), pointer1
```

复制

这里需要注意，如果一个指针的类型是 `void *`，那么是不能对其进行算术操作的，因为我们没办法确定其大小。

我们不能够对空指针进行解引用，来看一个例子：

```
int *ptr1 = malloc(sizeof(int));
*ptr1 = 0xdeadbeef;

int val1 = *ptr1;
int val2 = (int) *((char *) ptr1);
```

复制

那么 `val1` 和 `val2` 的值分别是什么呢？

```
val1` 的值比较简单，因为没有改动，所以就是 `0xdeadbeef`，不过 `val2` 的值就比较特别了，我们具体来看一看，首先指针 `ptr1` 被转换成了 `char` 指针，按照小端规则，指针指向的值变成了 `0xef`，解引用之后在转换成 `int` 类型，前面会补上 `ffffff`，最后就是 `0xffffffef
```

### Malloc

需要知道的概念：

- malloc / calloc / realloc
- free
- sbrk
- payload
- framentation (internal vs. external)
- colescing
  - Bi-directional
  - Immediate vs. Deferred

在 paylaod 比 block size 小的时候就会产生内部碎片，比方说

```
void *m1 = malloc(3);
void *m2 = malloc(3);
```

复制

因为 m1 和 m2 都需要以 8 bytes 对齐，所以都会有 5 个 bytes 的内部碎片。

实现是需要考虑的问题有：

- 怎么知道 block 在哪里
- 怎么知道 block 多大
- 怎么知道 block 是否 free
- 注意：不能缓存 malloc/free 调用，必须实时处理
- 注意：调用 free 的时候只传入一个指针，不会有表示 size 的参数
- 我们需要一个数据结构来存储关于 block 的相关信息

这个数据结构需要做到：

- block 的位置，block 的大小，以及 block 是否 free
- 在 malloc/free 调用的时候，需要能够改变这个数据结构
- 需要通过这个数据结构找到下一个合适的 block
- 能够快速标记一个 block 是 free 还是 allocated
- 能够检测是否有足够空间

具体怎么实现就需要自己思考了，唯一需要注意的是内存就是我们存放这些信息的地方。

## 总结

这次实验是整个系列七次中最难的一次，如果说其他的实验更像是拆炸弹拼拼图或者盖房子的话，那么这次就像是拿着手术刀做心脏手术，一着不慎，满盘皆输。无论是对于每个比特的操作，还是在有限的内存空间中实现高效的数据结构，都需要非常高的操作精度，也就是说，我们一要知道自己应该做什么，二要知道怎么做，最后还要做得好。
