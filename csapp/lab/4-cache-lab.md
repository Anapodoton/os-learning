[Cache Lab - 实现一个缓存系统来加速计算](http://wdxtub.com/csapp/thick-csapp-lab-4/2016/04/16/)

<!--ts-->
      * [任务目标](#任务目标)
      * [C 语言复习](#c-语言复习)
      * [第一部分 缓存模拟器](#第一部分-缓存模拟器)
         * [任务目标](#任务目标-1)
         * [一些提示](#一些提示)
      * [第二部分 优化矩阵转制](#第二部分-优化矩阵转制)
         * [任务目标](#任务目标-2)
         * [缓存分析](#缓存分析)
         * [一些提示](#一些提示-1)
      * [总结](#总结)

<!-- Added by: anapodoton, at: Thu Feb  6 23:00:48 CST 2020 -->

<!--te-->

## 任务目标

这次实验的任务很明确，就是制作自己的缓存系统，具体来说是

- 实现一个缓存模拟器，根据给定的 trace 文件来输出对应的操作
- 利用缓存机制加速矩阵运算

我们需要修改的是 `csim.c`（需要自己创建） 和 `trans.c`。编译的时候只需要简单 `make clean` 和 `make`，然后就可以进行测试了。

## C 语言复习

- 注意代码风格，不要写糟糕的代码
- 小心隐式类型转换
- 小心未定义的行为
- 小心内存泄露
- 宏和指针计算很容易出错

**例 1**

```
int foo(unsigned int u) {
    return (u > -1) ? 1 : 0;
}
```

复制

因为 `u` 是无符号整型，所以在比较的时候 -1 也会按照无符号整型来处理，于是实际的比较相当于 `u > int_max`，使得这个函数总是会返回 0。

**例 2**

```
int main() {
    int* a = malloc(100*sizeof(int));
    for (int i=0; i < 100; i++) {
        a[i] = i / a[i];
    }
    free(a);
    return 0;
}
```

复制

这里 `a` 中的值都没有进行初始化，所以 main 函数的行为是未定义的。

**例 3**

```
int main() {
    char w[strln("C programming")];
    strcpy(w, "C programming");
    printf("%s\n", w);
    return 0;
}
```

复制

`strlen` 返回的长度是不包括最后的 `\0` 的，写入的时候会越界。

**例 4**

```
struct ht_node {
    int key;
    int data;
};
typedef struct ht_node* node;

node makeNnode(int k, int e) {
    node curr = malloc(sizeof(node));
    curr->key = k;
    curr->data = e;
}
```

复制

这里把 `node` 定义为一个指针，并不是指向一个结构体。

**例 5**

```
char *strcdup(int n, char c){
    char dup[n+1];
    int i;
    for (i = 0; i < n; i++)
        dup[i] = c;
    dup[i] = '\0';
    char *A = dup;
    return A;
}
```

复制

`strcdup` 函数返回了一个分配在栈中的指针，函数返回之后地址 `A` 可能会被抹掉。

**例 6**

```
#define IS_GREATER(a, b) a > b
inline int isGreater(int a, int b) {
    return a > b ? 1 : 0;
}
int m1 = IS_GREATER(1, 0) + 1;
int m2 = isGreater(1, 0) + 1;
```

复制

```
IS_GREATER` 是一个没有带括号的宏，所以 `m1` 的值相当于 `1 > 0+1 = 0
```

**例 7**

```
#define NEXT_BYTE(a) ((char *)(a + 1));

int a1 = 54; // &a1 = 0x100
long long a2 = 42; // &a2 = 0x200
void* b1 = NEXT_BYTE(&a1);
void* b2 = NEXT_BYTE(&a2);
```

复制

- 这里 `b1` 指向 `0x104`
- 这里 `b2` 指向 `0x208`

会根据类型的不同，决定下一个 byte 的起始位置。

注意提交的代码列数不要超过 80，会人工检查的。

**GDB 命令**

- `gdbtui `
- `layout split`

![img](https://wdxtub.com/images/14557338869148.jpg)

**valgrind**

可以用来查找

- 内存泄露
- 其他内存错误
- 内存损害

使用 `gcc -g` 可以列出内存泄露的具体行数

使用 `valgrind --leak-check=full` 可以查看全部详细信息

## 第一部分 缓存模拟器

这里需要注意的是，缓存模拟器并不是缓存！我们不需要存储具体的内存内容，也就是说，块偏移量不重要（也就是最后的 b bits 我们不需要在意），只需要计算 hit, miss 和 eviction 的次数即可。

我们的缓存模拟器需要在不同的 s, b, E 设定下正常工作，使用的替换策略是 LRU(Least Recently Used)，这里可以考虑用队列或者时间戳的方式来记录。

### 任务目标

先来了解一下这次用作输入数据的 trace 文件，可以使用 `valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l` 来体验一下，具体做的事情，就是把执行 `ls -l` 这个命令时访问内存的日志输出到终端中，大概是这样的：

![img](https://wdxtub.com/images/14557449805626.jpg)

每一条对内存访问的记录格式是 `[空格]操作符 地址,大小`，以 `I` 开头的是载入指令的记录，不算在内存访问中。

- M 表示数据修改，需要一次载入 + 一次存储，也就是相当于两次访问
- S 表示数据存储
- L 表示数据载入
- 地址指的是一个 64 位的 16 进制内存地址
- 大小表示该操作内存访问的字节数

我们要做的实际是读入 `valgrind` 生成的 trace 日志，并统计出 hit, miss 和 eviction 的次数。同时已经给出了一个参考程序 `csim-ref`，于是任务就变成山寨一个这样的程序，比如：

![注意两种不同的模式](https://wdxtub.com/images/14557460323055.jpg)

具体用法如下：

![注意各个参数的含义](https://wdxtub.com/images/14557462175638.jpg)

其他的就是概念的理解和实现了，这里给出几个关键问题，大家可以从这里找到思路：

1. 如何从命令中拿到所需的参数
2. 如何从文件中读入内容
3. 如何进行 cache 的存储
4. 如何进行错误处理
5. 如何保证两种模式输出

因为已经给出了参考输出，所以对着一点一点做，应该没有太多问题。

### 一些提示

我们可以用一个 `cache_line` 的结构体来保存每个 cache line 的信息，如：

```
typedef struct
{
    int valid;
    int tag;
    int time_stamp;
} cache_line;
```

复制

而对应到我们的缓存模拟器，可以用一个二维数组来存储并模拟，如：`cache[S][E]`。这里 $S=2^s$，也就是 set 的个数，E 是一个 set 里有多少个 cache line。

因为这次测试的输入是如下所示的命令：

```
./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>
```

复制

也就是说我们的程序需要能够读取命令行参数。这时候可以使用 `getopt()` 来完成，使用前注意包括 `#include `，如果不在 CMU 的机器上运行，还需要加上 `#include `。

具体的使用方法参见 `man 3 getopt` 或 [这里](http://www.gnu.org/software/libc/manual/html_node/Getopt.html)。当然，还需要考虑如何处理非法输入。

有了命令行参数，我们还需要对应读入 trace 文件的内容，可以使用 `fscanf` 来完成这个任务，具体的使用可以参考 `man fscanf` 或 [这里](http://crasseux.com/books/ctutorial/fscanf.html)

因为我们需要根据传入的参数来创建 cache，所以需要动态分配空间，可以使用 `malloc` 在堆上完成这个任务。但是需要注意 `malloc` 的内容一定要 `free` 掉，不然就会造成内存泄露。另外，不是自己 `malloc` 的内存，也不要去 `free` 掉。一个例子：

```
some_pointer_you_malloced = malloc(sizeof(int));
Free(some_pointer_you_malloced);
```

复制

## 第二部分 优化矩阵转制

### 任务目标

测试的矩阵尺寸为：

- 32 x 32
- 64 x 64
- 61 x 67

缓存的指标：

- 有 1KB 大小的缓存
- 是 directly mapped，也就是 E=1
- Block size 为 32 字节，也就是 b=5
- 一共有 32 个 set，也就是 s=5

可能出乎大家意料，最困难的其实是 64 x 64，因为对于 miss 数量的要求非常严苛！

### 缓存分析

为了加深印象，还是具体分析一下使用的策略。61 x 67 因为不规则，所以基本的策略就是用不同的分块大小去实验，我一路从 8 测试到 24，最后选了个看起来最顺眼的。这里主要还是分析 32 x 32 和 64 x 64 的。

根据给出的缓存大小，可以知道，一个 block 可以放 8 个 int 值，那么对于 32 x 32 矩阵，有下面的图：

![32 x 32](https://wdxtub.com/images/14557681819928.jpg)

这里的数字表示对应的值会存在缓存的哪个 set 中，我们可以看到第九行和第一行会冲突，但是如果我们分成 8 x 8 的小块，就可以保证尽可能利用到缓存的特性（读取第 1 个的时候后面 7 个也已经载入缓存中），于是简单分块就可以解决 32 x 32 的矩阵（要求是 miss 数量不超过 300）

但是对于 64 x 64，情况就不一样了，首先是 miss 的数量限制得比较严格，不超过 1300，然后是缓存中的排列也有一些变化，如图：

![64 x 64](https://wdxtub.com/images/14557683448384.jpg)

因为宽度的变化，现在第 5 行就会和第 1 行冲突，所以如果我们还用原来的 8 x 8，肯定是不行的。那么如果用 4 x 4 呢？经过尝试之后，发现超过要求的 1300 还是比较多的。问题在哪里？一是因为每次读取 4 个数字，我们实际只用了 4 个，二是宽度改变带来的 conflict miss 仍旧没有很好解决。

题目中说不能使用超过 12 个变量，有 4 个需要作为遍历矩阵的索引值，另外八个是不是可以做一点文章呢？答案是肯定的。

我们依然可以按照原来的 8 x 8 来进行处理，只是在每个 8 x 8 中，还需要进行分块。首先是左上角的 4 x 4，这里没有冲突，都会在缓存中，所以按照正常的分块算法处理。比较有技巧的是右上角和左下角，这里我们会发现重复访问会造成很多的 conflict miss，这个时候就可以利用 4 个本地变量作为 buffer，先存起来，以后再用。这样就利用了每次读入 8 个的优势，并且避开了因为地址冲突而导致的 conflict miss 的问题。

### 一些提示

矩阵转制并不是特别复杂的操作，但是如何利用缓存来尽可能提高速度，就有很多的优化空间了。和矩阵相乘中使用的 cache blocking 方法一样，在这里我们同样可以利用 blocking 的方法来提高效率，可能需要尝试不同的大小，以找到最合适的尺寸。

最好在编译的时候加上 `-Werror`，这样就不会放过任何一个 warning。如果使用的函数缺少头文件，可以通过 `man ` 找到相关头文件

因为只会跑 32x32, 64x64, 61x67 这三个测试，所以可以硬编码来进行检测，针对不同的矩阵大小进行优化。

都写完之后可以利用 `./driver.py` 进行完整的测试，最后提交自动打包好的 tar 压缩包到 autolab 即可。

## 总结

被汇编语言虐了两周之后，回过头来写 C 语言，真的感觉从地狱到了天堂。虽然降低了语言的复杂度，但是对于概念的理解有了更高的要求。回过头来想想，正因为隐藏了足够多的细节，更高层级的抽象才得以方便实现。

缓存的整个设计非常精妙，仅仅利用 locality 一个特性，几乎是用最小的成本达到了最好的性能。

> The memory hierarcy creates a large pool of storage that costs as much as the cheap storage near the bottom, but that serves data to programs at the rate of the fast storage near the top.

带着镣铐跳舞，精髓。
