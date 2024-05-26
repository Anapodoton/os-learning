# **了解CPU硬件**

![图片](https://mmbiz.qpic.cn/mmbiz/BBjAFF4hcwqhHYhLNtUmdqHF13OkLTl4uiaDg4bu4W5ibpIibtH95EekswaqR2CEcFLQuKBFrDibBHOCaxUZgWvZcw/640?wx_fmt=other&wxfrom=5&wx_lazy=1&wx_co=1)

# 内核 CPU 开销

进程切换一次开销：**2.7-5.48us左右**。

软中断CPU开销大约3.4us左右。

系统调用一次的开销200ns（执行1000 条左右的指令），函数调用的开销不到 1ns 。

# **用户态CPU开销分析**

C 语言的一次函数调用开销在 0.4ns 左右，需要的 CPU指令数是8 个（整数运算，分支转移，加载存储，寄存器访问，系统调用，原子指令）。

php 语言的一次函数调用开销在52ns左右。

# 参考

https://mp.weixin.qq.com/s/G30VtOIYjx2Wa54xlO7udw