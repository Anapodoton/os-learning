[ Bomb Lab - 汇编，栈帧与 gdb](http://wdxtub.com/csapp/thick-csapp-lab-2/2016/04/16/)

<!--ts-->
      * [任务目标](#任务目标)
      * [热身](#热身)
         * [汇编复习](#汇编复习)
         * [GDB 介绍](#gdb-介绍)
      * [第一关](#第一关)
      * [第二关](#第二关)
      * [第三关](#第三关)
      * [第四关](#第四关)
      * [第五关](#第五关)
      * [第六关](#第六关)
      * [秘密关卡](#秘密关卡)
      * [总结](#总结)

<!-- Added by: anapodoton, at: Thu Feb  6 23:00:35 CST 2020 -->

<!--te-->

## 任务目标

这次的任务很『简单』，一共有七关，六个常规关卡和一个隐藏关卡，每次我们需要输入正确的『拆弹密码』才能进入下一关，而具体的『拆弹密码』藏在汇编代码中。进入隐藏关卡的方式也在其中！这就需要我们一点一点探索蛛丝马迹了。

## 热身

### 汇编复习

想要完成拆弹任务，不但需要理解不同寄存器的常用方法，也要弄明白具体的操作符是什么意思：

|   类型   |     语法      |                      例子                      |             备注             |
| :------: | :-----------: | :--------------------------------------------: | :--------------------------: |
|   常量   | 符号`$` 开头  |               `$-42`, `$0x15213`               | 一定要注意十进制还是十六进制 |
|  寄存器  | 符号 `%` 开头 |                 `%esi`, `%rax`                 |     可能存的是值或者地址     |
| 内存地址 |  括号括起来   | `(%rbx)`, `0x1c(%rax)`, `0x4(%rcx, %rdi, 0x1)` |   括号实际上是去寻址的意思   |

一些汇编语句与实际命令的转换：

|            指令             |         效果         |
| :-------------------------: | :------------------: |
|      `mov %rbx, %rdx`       |     `rdx = rbx`      |
|      `add (%rdx), %r8`      | `r8 += value at rdx` |
|        `mul $3, %r8`        |      `r8 *= 3`       |
|        `sub $1, %r8`        |        `r8--`        |
| `lea (%rdx, %rbx, 2), %rdx` | `rdx = rdx + rbx*2`  |

比较与跳转是拆弹的关键，基本所有的字符判断就是通过比较来实现的，比方说 `cmp b,a` 会计算 `a-b` 的值，`test b, a` 会计算 `a&b`，注意运算符的顺序。例如

```
cmpl %r9, %r10
jg   8675309
```

复制

等同于 `if %r10 > %r9, jump to 8675309`

各种不同的跳转：

|  指令   |         效果         | 指令 |            效果             |
| :-----: | :------------------: | :--: | :-------------------------: |
|   jmp   |     Always jump      |  ja  |  Jump if above(unsigned >)  |
|  je/jz  |  Jump if eq / zero   | jae  |    Jump if above / equal    |
| jne/jnz | Jump if !eq / !zero  |  jb  |  Jump if below(unsigned <)  |
|   jg    |   Jump if greater    | jbe  |    Jump if below / equal    |
|   jge   | Jump if greater / eq |  js  | Jump if sign bits is 1(neg) |
|   jl    |     Jump if less     | jns  | Jump if sign bit is 0 (pos) |
|   jle   |  Jump if less / eq   |  x   |              x              |

举几个例子

```
cmp $0x15213, %r12
jge deadbeef
```

复制

若 `%r12 >= 0x15213`，则跳转到 `0xdeadeef`

```
cmp %rax, %rdi
jae 15213b
```

复制

如果 `%rdi` 的无符号值大于等于 `%rax`，则跳转到 `0x15213b`

```
test %r8, %r8
jnz (%rsi)
```

复制

如果 `%r8 & %r8` 不为零，那么跳转到 `%rsi` 存着的地址中。

```
# 检查符号表
# 然后可以寻找跟 bomb 有关的内容
objdump -t bomb | less 

# 反编译
# 搜索 explode_bomb
objdump -d bomb > bomb.txt

# 显示所有字符
strings bomb | less
```

复制

### GDB 介绍

```
gdb bomb

# 获取帮助
help

# 设置断点
break explode_bomb
break phase_1

# 开始运行
run

# 检查汇编 会给出对应的代码的汇编
disas 

# 查看寄存器内容
info registers

# 打印指定寄存器
print $rsp

# 每步执行
stepi

# 检查寄存器或某个地址
x/4wd $rsp
```

复制

用 ctl+c 可以退出，每次进入都要设置断点（保险起见），炸弹会用 `sscanf` 来读取字符串，了解清楚（感谢网友十六夜砕月指正）到底需要输入什么。

## 第一关

我们先来看看符号表 `objdump -t bomb | less`

![img](https://wdxtub.com/images/14544652386936.jpg)

就会发现这是天书，什么鬼！不过既然我们是要拆炸弹，不如就搜索一下 `bomb`，看看有没有什么线索。在 less 下输入 `/bomb` 然后不断回车，看到以下这些关键字：

- `bomb.c`
- `initialize_bomb_solve`
- `explode_bomb`
- `bomb_id`
- `initialize_bomb`

看来这里唯一有用的就是这个 `explode_bomb`，顾名思义，估计是在拆弹失败的时候用来爆炸的，所以我们可以直接设个断点，暂停运行，不让它爆炸。

然后我们就可以反编译一下炸弹看看到底里面是怎么回事了：`objdump -d bomb > bomb.txt`

![img](https://wdxtub.com/images/14544657722862.jpg)

比方说我们大概可以看出来，这里就是 main 函数执行的地方了。往下找找就可以看到第一阶段的代码了，如下：

```
0000000000400fb0 <phase_1>:
  400fb0:	48 83 ec 08          sub    $0x8,%rsp
  400fb4:	be f0 27 40 00       mov    $0x4027f0,%esi
  400fb9:	e8 72 04 00 00       callq  401430 <strings_not_equal>
  400fbe:	85 c0                test   %eax,%eax
  400fc0:	74 05                je     400fc7 <phase_1+0x17>
  400fc2:	e8 3d 07 00 00       callq  401704 <explode_bomb>
  400fc7:	48 83 c4 08          add    $0x8,%rsp
  400fcb:	c3                   retq
```

复制

先来简单观察下这段程序在做什么，`callq` 的两行就是调用 `strings_not_equal` 和 `explode_bomb` 这两个函数的，而这里 `%esi` 对应的是第二个参数，第一个参数呢？当然就是我们拆弹时需要输入的字符串了。之后的 `test` 是用来判断函数的返回值 `%eax` 是否为 0， 如果为 0 则进行跳转，否则炸弹爆炸，所以我们实际上要做的，就是看看 `$0x4027f0` 这个地址里对应存放的是什么字符串，也就是拆炸弹的关键了。

先 `gdb bomb`，然后设置断点 `break explode_bomb` 和 `break phase_1`

![img](https://wdxtub.com/images/14544673817513.jpg)

接着运行 `run`，就会在断点处停下，这里会先让我们输入第一关的密码，随便输入一个抵达断点再说。

![img](https://wdxtub.com/images/14544675211698.jpg)

我们现在到断点了，可以利用 `disas` 来看看对应的汇编代码，其实就和我们之前反汇编出来的一致：

![img](https://wdxtub.com/images/14544676048852.jpg)

然后我们看看寄存器里的内容 `info registers`:

![img](https://wdxtub.com/images/14544677052252.jpg)

诶，不是说我们输入的字符串（也就是 abc）会存放在 `eax` 里面吗？怎么列表里没有？其实 `eax` 是 `rax` 的低位，我们可以直接利用 `print $eax` 把它打印出来，就会发现，是一个地址，我们再用 `x/s $eax` 就可以看到我们刚才输入的字符串了。

![img](https://wdxtub.com/images/14544680308984.jpg)

然后我们继续回到汇编语句，用 `stepi` 来逐步执行，就可以看到箭头的变化：

![img](https://wdxtub.com/images/14544681259153.jpg)

这里我们看到 `mov` 语句已经执行完成了，那么好，可以直接用 `x $esi` 来看看传进去的到底是什么内容了：

![img](https://wdxtub.com/images/14544682048627.jpg)

Bingo！这就是第一关的答案了，赶紧记下来吧！（注意每个人的都是不同的）

```
Why make trillions when we could make... billions?
```

然后输入 `quit` 退出 gdb，新建一个文本文件 `touch sol.txt`，方便以后输入答案。

## 第二关

这次因为我们有了输入，所以需要在进入 gdb，设置好断点后，设置命令参数

![img](https://wdxtub.com/images/14544708331343.jpg)

然后试着运行一下，在 `phase_1` 停住了，然后我们输入 `continue`，来看看答案到底对不对，如果正确，应该会在 `phase_2` 停住，如果错误，则会在 `explode_bomb` 停住。

![img](https://wdxtub.com/images/14544713961334.jpg)

然后就发现，第一关已经顺利完成了，然后挑战第二关。我们再随便输入一些内容，触发 `phase_2` 的断点。（这次输入 abcd，结果如下）

![img](https://wdxtub.com/images/14544714769384.jpg)

这一段代码比较长，我们还是来看看到底在做什么。从函数名可以得知，这一次要读入六个数字 `read_six_numbers`。

从 `cmpl $0x1, (%rsp)`（感谢网友十六夜砕月指正） 看出第一个数字一定是 1，然后跳转到 +24 的位置，然后把 1 移动到 `%ebx` 中，跳转到 +57 的位置，然后和 5 进行比较，因为 1 比 5 小，所以会跳转到 +31 的位置。

接着是 `movslq` 语句，这个语句是带符号地把第一个寄存器扩展并复制到第二个寄存器中，所以现在 `%rdx` 的值也是 1。`lea` 之后 `%eax` 等于 0，然后用 `cltq` 扩展到 64 位（也就是 `%rax` 等于 0），接着的语句相当于是 `%eax = (%rsp) + 4 * %rax` 即 `%eax` 等于 1。然后与自己相加等于乘以 2，现在 `%eax` 等于 2，然后等于是判断第二个参数(`(%rsp, %rdx, 4)`)和 2 是否相等，所以第二个数字是 2。

然后进行循环的累加并返回到 +31 的位置，继续循环。接着就是类似的操作了，最后分析可以得到每次增大一倍，答案就是 1 2 4 8 16 32。

## 第三关

第三关的代码很长，而且猛看上去，到处都可能触发炸弹。

```
0000000000401010 <phase_3>:
  401010:	48 83 ec 18          sub    $0x18,%rsp
  401014:	4c 8d 44 24 0c       lea    0xc(%rsp),%r8
  401019:	48 8d 4c 24 07       lea    0x7(%rsp),%rcx
  40101e:	48 8d 54 24 08       lea    0x8(%rsp),%rdx
  401023:	be 4e 28 40 00       mov    $0x40284e,%esi
  401028:	b8 00 00 00 00       mov    $0x0,%eax
  40102d:	e8 7e fc ff ff       callq  400cb0 <__isoc99_sscanf@plt>
  // %eax为sscanf的返回值，正确值为3
  401032:	83 f8 02             cmp    $0x2,%eax
  401035:	7f 05                jg     40103c <phase_3+0x2c>
  401037:	e8 c8 06 00 00       callq  401704 <explode_bomb>
  // 说明第一个数小于等于7
  40103c:	83 7c 24 08 07       cmpl   $0x7,0x8(%rsp)
  401041:	0f 87 fc 00 00 00    ja     401143 <phase_3+0x133>
  401047:	8b 44 24 08          mov    0x8(%rsp),%eax
  // 跳转，0x402860为起始地址，%rax为偏移
  40104b:	ff 24 c5 60 28 40 00 jmpq   *0x402860(,%rax,8)
  401052:	b8 6e 00 00 00       mov    $0x6e,%eax
  401057:	81 7c 24 0c df 00 00 cmpl   $0xdf,0xc(%rsp)
  40105e:	00 
  40105f:	0f 84 e8 00 00 00    je     40114d <phase_3+0x13d>
  401065:	e8 9a 06 00 00       callq  401704 <explode_bomb>
  40106a:	b8 6e 00 00 00       mov    $0x6e,%eax
  40106f:	e9 d9 00 00 00       jmpq   40114d <phase_3+0x13d>
  401074:	b8 6a 00 00 00       mov    $0x6a,%eax
  401079:	81 7c 24 0c 01 03 00 cmpl   $0x301,0xc(%rsp)
  401080:	00 
  401081:	0f 84 c6 00 00 00    je     40114d <phase_3+0x13d>
  401087:	e8 78 06 00 00       callq  401704 <explode_bomb>
  40108c:	b8 6a 00 00 00       mov    $0x6a,%eax
  401091:	e9 b7 00 00 00       jmpq   40114d <phase_3+0x13d>
  401096:	b8 63 00 00 00       mov    $0x63,%eax
  40109b:	81 7c 24 0c 1d 01 00 cmpl   $0x11d,0xc(%rsp)
  4010a2:	00 
  4010a3:	0f 84 a4 00 00 00    je     40114d <phase_3+0x13d>
  4010a9:	e8 56 06 00 00       callq  401704 <explode_bomb>
  4010ae:	b8 63 00 00 00       mov    $0x63,%eax
  4010b3:	e9 95 00 00 00       jmpq   40114d <phase_3+0x13d>
  4010b8:	b8 70 00 00 00       mov    $0x70,%eax
  4010bd:	81 7c 24 0c 16 02 00 cmpl   $0x216,0xc(%rsp)
  4010c4:	00 
  4010c5:	0f 84 82 00 00 00    je     40114d <phase_3+0x13d>
  4010cb:	e8 34 06 00 00       callq  401704 <explode_bomb>
  4010d0:	b8 70 00 00 00       mov    $0x70,%eax
  4010d5:	eb 76                jmp    40114d <phase_3+0x13d>
  4010d7:	b8 77 00 00 00       mov    $0x77,%eax
  4010dc:	81 7c 24 0c cd 00 00 cmpl   $0xcd,0xc(%rsp)
  4010e3:	00 
  4010e4:	74 67                je     40114d <phase_3+0x13d>
  4010e6:	e8 19 06 00 00       callq  401704 <explode_bomb>
  4010eb:	b8 77 00 00 00       mov    $0x77,%eax
  4010f0:	eb 5b                jmp    40114d <phase_3+0x13d>
  4010f2:	b8 70 00 00 00       mov    $0x70,%eax
  4010f7:	81 7c 24 0c 9a 00 00 cmpl   $0x9a,0xc(%rsp)
  4010fe:	00 
  4010ff:	74 4c                je     40114d <phase_3+0x13d>
  401101:	e8 fe 05 00 00       callq  401704 <explode_bomb>
  401106:	b8 70 00 00 00       mov    $0x70,%eax
  40110b:	eb 40                jmp    40114d <phase_3+0x13d>
  40110d:	b8 74 00 00 00       mov    $0x74,%eax
  401112:	81 7c 24 0c 13 01 00 cmpl   $0x113,0xc(%rsp)
  401119:	00 
  40111a:	74 31                je     40114d <phase_3+0x13d>
  40111c:	e8 e3 05 00 00       callq  401704 <explode_bomb>
  401121:	b8 74 00 00 00       mov    $0x74,%eax
  401126:	eb 25                jmp    40114d <phase_3+0x13d>
  401128:	b8 79 00 00 00       mov    $0x79,%eax
  40112d:	81 7c 24 0c 3b 01 00 cmpl   $0x13b,0xc(%rsp)
  401134:	00 
  401135:	74 16                je     40114d <phase_3+0x13d>
  401137:	e8 c8 05 00 00       callq  401704 <explode_bomb>
  40113c:	b8 79 00 00 00       mov    $0x79,%eax
  401141:	eb 0a                jmp    40114d <phase_3+0x13d>
  401143:	e8 bc 05 00 00       callq  401704 <explode_bomb>
  401148:	b8 63 00 00 00       mov    $0x63,%eax
  40114d:	3a 44 24 07          cmp    0x7(%rsp),%al
  401151:	74 05                je     401158 <phase_3+0x148>
  401153:	e8 ac 05 00 00       callq  401704 <explode_bomb>
  401158:	48 83 c4 18          add    $0x18,%rsp
  40115c:	c3                   retq
```

复制

可以看到一开始的 `0x40284e` 非常突兀，我们可以打印它的值：

![img](https://wdxtub.com/images/14544764186906.jpg)

就可以发现本题要输入的格式了，接着看到这么多分片的语句，非常类似于我们的 switch 语句。所以第一个数字是用来进行跳转的，如下图所示

![img](https://wdxtub.com/images/14544776193511.jpg)

比方说如果输入 0，那么就直接执行下一条 mov 语句，然后是比较第三个参数是否和 0xdf 相等，所以我们知道第三个参数是 223（如果第一个参数是 0）的话。如果一切正常，那么就会跳转到 +317 的位置，也就是：

![img](https://wdxtub.com/images/14544777298994.jpg)

我们只要搞清楚 `%al` 里面的值是什么就好（会和第二个参数进行比较），具体的值，其实就是前面 mov 语句读入的 0x6e（110），对应的字符是 n。

所以答案是 0 n 223，当然选择不同的分支就有不同的答案，其他分支的分析也都是类似的。运行一下，就可以发现这一关又过了

![img](https://wdxtub.com/images/14544780985022.jpg)

## 第四关

这一关要涉及到一个函数，我们先把 `phase_4` 的代码弄出来：

![img](https://wdxtub.com/images/14544781877930.jpg)

而对应 func4 的汇编代码是：

```
000000000040115d <func4>:
  40115d:	41 54                push   %r12
  40115f:	55                   push   %rbp
  401160:	53                   push   %rbx
  401161:	89 fb                mov    %edi,%ebx
  401163:	85 ff                test   %edi,%edi
  401165:	7e 24                jle    40118b <func4+0x2e>
  401167:	89 f5                mov    %esi,%ebp
  401169:	89 f0                mov    %esi,%eax
  40116b:	83 ff 01             cmp    $0x1,%edi
  40116e:	74 20                je     401190 <func4+0x33>
  401170:	8d 7f ff             lea    -0x1(%rdi),%edi
  
  401173:	e8 e5 ff ff ff       callq  40115d <func4>
  401178:	44 8d 24 28          lea    (%rax,%rbp,1),%r12d
  40117c:	8d 7b fe             lea    -0x2(%rbx),%edi
  40117f:	89 ee                mov    %ebp,%esi
  
  401181:	e8 d7 ff ff ff       callq  40115d <func4>
  401186:	44 01 e0             add    %r12d,%eax
  401189:	eb 05                jmp    401190 <func4+0x33>
  40118b:	b8 00 00 00 00       mov    $0x0,%eax
  401190:	5b                   pop    %rbx
  401191:	5d                   pop    %rbp
  401192:	41 5c                pop    %r12
  401194:	c3                   retq
```

复制

和上一关类似，我们可以先从 `0x402b56` 这个地址获取到具体的输入格式：

![img](https://wdxtub.com/images/14545016028067.jpg)

可以看到这一关我们需要输入两个数字，在检查输入参数的个数是否为 2 个之后，先判断第一个参数是否小于等于 1，如果是就爆炸，所以第一个数字需要大于等于 1。然后判断第一个参数是否小于等于 4，这里需要满足这个条件，所以第一个参数可能的取值（目前来看）是 2, 3, 4。

然后是把函数调用的参数传到 `%edi` 中，也就是说传入的参数是 9 和 2（我们输入的第 2 参数），然后需要用我们之前输入的第 1 个参数来和函数的返回值比较，于是我们需要弄明白这个递归函数在做什么。

一开始的 jle 跳转相当于是递归的退出条件，可以看到只有当 `%edi` 为 0 时，才会退出（现在 `%edi` 的值是 9），接着把我们输入的第一个参数存到 `%ebp` 和 `%eax` 中。之后又是跳转，因为 `%edi` 的值不等于 1，所以会继续执行。`lea` 那一句的左右就是让 `%edi` 的值减一（变成 8），然后开始递归调用。（可能需要 `delete [断点编号]` 方便调试）

递归部分有两个参数，第一个参数是程序中给出的 9，第二个参数是我们之前输入的参数中的第二个（也就是说可以是 2/3/4），程序的返回值会和我们输入的第一个参数进行比较。整个递归函数可以转换成如下的语句：

```
int f(x, y){
    if (x == 0) return 0;
    if (x == 1) return y;
    return f(x-1,y) + f(x-2,y) + y;
}
```

复制

所以其中一个答案就是 `264 3`，测试一下，发现顺利通过！

![img](https://wdxtub.com/images/14545084801542.jpg)

## 第五关

根据代码来判断，我们要输入的是一个长度为 6 的字符串（+9 那一句）。然后会经过一系列复杂的跳转，匹配的话就正常返回。

![img](https://wdxtub.com/images/14545086537046.jpg)

代码不算很长，通过第一个验证（长度位 6）之后，会把 `%edx` 和 `%eax` 都赋值为 0，然后用 `%eax` 和 5 进行比较，相当于是循环的计数，于是我们跳转回 +31 句。这里有两个新指令 `movslq`（扩展到64位，但是不填充符号位） 和 `movzbl`（扩展到 32 位，填充 0），然后我们取得到的值的最后四位。

接着出现了一个地址，我们来看看地址里面的内容是什么

![img](https://wdxtub.com/images/14545104158997.jpg)

可以看到是一个数组，而后面的代码等于是根据我们的输入在这个数组中选取数字进行累加，选取的规则是用输入字符的最低四位，最后的结果要和 `0x34` 进行比较，也就是需要六个数字加起来是 52。

那么这六个数字可以怎么凑到 52 呢？我随便凑了一下，16+16+10+6+2+2=52。对应的偏移量是 5, 5, 1, 2, 0, 0，然后我们找一个 ASCII 表：

![img](https://wdxtub.com/images/14545105868475.jpg)

按照最低位的取值来选符合条件的字母，我这里挑了 `eeabpp`，然后测试一下，再次通关！

![img](https://wdxtub.com/images/14545106954452.jpg)

## 第六关

最后一关！代码非常长：

![img](https://wdxtub.com/images/14545109182445.jpg)
![img](https://wdxtub.com/images/14545113919133.jpg)

一开始读取六个数字进来，后面一顿疯狂跳转，到底在做什么呢？我用笨办法，一句一句先翻译出来，注意跳转的时候最好标记一下

```
Dump of assembler code for function phase_6:
=> 0x40122c <+0>:     push   %r12
   0x40122e <+2>:     push   %rbp
   0x40122f <+3>:     push   %rbx
   0x401230 <+4>:     sub    $0x50,%rsp
   0x401234 <+8>:     mov    %rsp,%rsi
   0x401237 <+11>:    callq  0x40173a <read_six_numbers>

   0x40123c <+16>:    mov    $0x0,%ebp               # ebp = 0
   0x401241 <+21>:    jmp    0x40127d <phase_6+81>

   0x401243 <+23>:    movslq %ebp,%rax
   0x401246 <+26>:    mov    (%rsp,%rax,4),%eax      # 取以 rsp 开头第 rax 个数，放到 eax 中
   0x401249 <+29>:    sub    $0x1,%eax               # eax -= 1
   0x40124c <+32>:    cmp    $0x5,%eax
   0x40124f <+35>:    jbe    0x401256 <phase_6+42>   # 小于等于 5 则跳转
   0x401251 <+37>:    callq  0x401704 <explode_bomb>

   0x401256 <+42>:    lea    0x1(%rbp),%r12d         # r12d(地址) = rbp 存放的地址 + 0x1
   0x40125a <+46>:    mov    %r12d,%ebx
   0x40125d <+49>:    movslq %ebp,%rbp
   0x401260 <+52>:    jmp    0x401275 <phase_6+73>

   0x401262 <+54>:    movslq %ebx,%rax
   0x401265 <+57>:    mov    (%rsp,%rax,4),%eax      # 取以 rsp 开头的第 rax 个数，放到 eax 中
   0x401268 <+60>:    cmp    %eax,(%rsp,%rbp,4)
   0x40126b <+63>:    jne    0x401272 <phase_6+70>   # 不能相等
   0x40126d <+65>:    callq  0x401704 <explode_bomb>
   0x401272 <+70>:    add    $0x1,%ebx               # ebx += 1

   0x401275 <+73>:    cmp    $0x5,%ebx               # ebx 与 5 比较
   0x401278 <+76>:    jle    0x401262 <phase_6+54>   # 小于的时候跳转
   0x40127a <+78>:    mov    %r12d,%ebp              # ebp = r12d

   0x40127d <+81>:    cmp    $0x5,%ebp               # ebp 与 5 比较
   0x401280 <+84>:    jle    0x401243 <phase_6+23>   # 小于的时候跳转
   # 前面相当于判断输入的六个数是否一样，并且要小于 6
   0x401282 <+86>:    mov    $0x0,%esi               # esi = 0
   0x401287 <+91>:    jmp    0x4012af <phase_6+131>

   0x401289 <+93>:    mov    0x8(%rdx),%rdx          # rdx(地址) = rdx 存着的地址 + 0x8
   0x40128d <+97>:    add    $0x1,%eax               # eax += 1
   0x401290 <+100>:   jmp    0x40129f <phase_6+115>

   0x401292 <+102>:   mov    $0x1,%eax               # eax = 1
   0x401297 <+107>:   mov    $0x604300,%edx          # edx = 0x604300
   0x40129c <+112>:   movslq %esi,%rcx               # rcx = esi

   0x40129f <+115>:   cmp    %eax,(%rsp,%rcx,4)      # 以 rsp 开头的第 rcx 个数与 eax 比较
   0x4012a2 <+118>:   jg     0x401289 <phase_6+93>   # 大于的时候跳转
   0x4012a4 <+120>:   movslq %esi,%rax               # rax = esi
   0x4012a7 <+123>:   mov    %rdx,0x20(%rsp,%rax,8)  # 把 rdx 存着的地址放到某个位置
   0x4012ac <+128>:   add    $0x1,%esi               # esi += 1

   0x4012af <+131>:   cmp    $0x5,%esi               # esi 与 5 比较
   0x4012b2 <+134>:   jle    0x401292 <phase_6+102>  # 小于的时候跳转
   0x4012b4 <+136>:   mov    0x20(%rsp),%rbx         # rbx = rsp 存着的地址 + 0x20 的新地址
   0x4012b9 <+141>:   mov    %rbx,%rcx               # rcx = rbx
   0x4012bc <+144>:   mov    $0x1,%eax               # eax = 1
   0x4012c1 <+149>:   jmp    0x4012d5 <phase_6+169>

   0x4012c3 <+151>:   movslq %eax,%rdx               # rdx = eax
   0x4012c6 <+154>:   mov    0x20(%rsp,%rdx,8),%rdx  # rdx = 以 rsp 开头加上 8 个 rdx 偏移再加 0x20
   0x4012cb <+159>:   mov    %rdx,0x8(%rcx)          # rcx 存着的地址 + 0x8 = rdx 存着的地址
   0x4012cf <+163>:   add    $0x1,%eax               # eax += 1
   0x4012d2 <+166>:   mov    %rdx,%rcx               # rcx = rdx

   0x4012d5 <+169>:   cmp    $0x5,%eax               # eax 与 5 比较
   0x4012d8 <+172>:   jle    0x4012c3 <phase_6+151>  # 小于的时候跳转
   0x4012da <+174>:   movq   $0x0,0x8(%rcx)          # rcx 存着的地址 + 0x8 = 0
   0x4012e2 <+182>:   mov    $0x0,%ebp               # ebp = 0
   0x4012e7 <+187>:   jmp    0x4012ff <phase_6+211>

   0x4012e9 <+189>:   mov    0x8(%rbx),%rax          # rax(地址) = rbx 存着的地址 + 0x8
   0x4012ed <+193>:   mov    (%rax),%eax             # eax = rax 地址中的值
   0x4012ef <+195>:   cmp    %eax,(%rbx)             # rbx 地址中的值与 eax 比较
   0x4012f1 <+197>:   jge    0x4012f8 <phase_6+204>  # 大于等于的时候跳转
   0x4012f3 <+199>:   callq  0x401704 <explode_bomb>
   0x4012f8 <+204>:   mov    0x8(%rbx),%rbx          # rbx(地址) = rbx 存着的地址 + 0x8
   0x4012fc <+208>:   add    $0x1,%ebp               # ebp += 1
   # 以上部分等于是用我们输入的顺序去验证内存中数据的顺序是否正确
   0x4012ff <+211>:   cmp    $0x4,%ebp               # ebp 与 4 比较
   0x401302 <+214>:   jle    0x4012e9 <phase_6+189>  # 小于等于的时候跳转
   0x401304 <+216>:   add    $0x50,%rsp
   0x401308 <+220>:   pop    %rbx
   0x401309 <+221>:   pop    %rbp
   0x40130a <+222>:   pop    %r12
   0x40130c <+224>:   retq
End of assembler dump.
```

复制

这里发现一个奇怪的地址 `0x604300`，我们来看看里面的内容是什么：

![img](https://wdxtub.com/images/14545155501283.jpg)

发现其实是一个结构体，类似于

```
struct {
    int value;
    int order;
    node* next;
} node;
```

复制

我们要做的，就是输入正确的 order（从大到小），这样程序在验证顺序的时候，就不会出问题，打印出来节点里的内容，人工排个序，就可以发现正确答案是 `6 2 1 5 4 3`。（感谢网友 `那影丶这光` 的更正）

![img](https://wdxtub.com/images/14545157165813.jpg)

拆弹任务成功！

## 秘密关卡

接着往反编译出来的源代码下看，发现还有一个隐藏关！但是之前过程中并没有任何需要给隐藏关输入的地方，那么就得先看看怎么进入隐藏关。在源代码中搜索 `secret_phase`，然后就可以发现，在 `phase_defused` 中会对其进行调用，那么我们就先来设个断点，看看能够怎么进去。

![img](https://wdxtub.com/images/14545166333931.jpg)

`phase_defused` 函数内容如下，我们在调用 `secret_phase` 的指令加个断点(`break *0x40191d`)，然后看看到底需要输入什么。

然后发现上面把两个参数放到了 `%edi` 中，我们来看看里面放了什么。结果发现是挑衅

![img](https://wdxtub.com/images/14545168650220.jpg)

继续往上（感谢网友十六夜砕月指正）找，在 `0x402ba0` 这个地址里，可以发现输入格式

![img](https://wdxtub.com/images/14545176516067.jpg)

但是并没有任何印象要输入这个，继续往上翻，发现又有一个地址：

![img](https://wdxtub.com/images/14545178110834.jpg)

唯一有类似输入格式的就是第四题，所以我们试试看在第四题的后面加上 `213rocks!`。果然，就可以进入到秘密关卡了。

![img](https://wdxtub.com/images/14545195897356.jpg)

这一段代码一开始就调用 `read_line`，然后会把内容用 `strtol` 转成十进制整数（感谢网友十六夜砕月指正），然后和 `0x3e8`（也就是 1000）进行比较，如果小于等于的话就执行 `fun7`，然后返回值需要等于 5，于是问题就变成，给定一个值，让 `fun7` 的输出为 5。我们就先来看看 `fun7` 具体做了什么。

![img](https://wdxtub.com/images/14545211010609.jpg)

一眼就能看出这是一个递归函数了，然后我们观察一下传进来作为第一个参数的地址 `0x604120`

![img](https://wdxtub.com/images/14545215838752.jpg)

虽然比较乱，但是可以看出是一棵树，有不同的层级。画出来的话大概是

```
              36
        /           \
      8               50
    /    \          /    \
   /      \        /      \
  6       22      45      107
 / \     /  \    /  \    /   \
1   7   20  35  40  47  99  1001
```

复制

递归实际上的逻辑类似于下面代码：

```
struct treeNode
{
    int data;
    struct treeNode* leftChild;
    struct treeNode* rightChild;
};

int fun7(struct treeNode* p, int v)
{
    if (p == NULL)
        return -1;
    else if (v < p->data)
        return 2 * fun7(p->leftChild, v);
    else if (v == p->data)
        return 0;
    else 
        return 2 * fun7(p->rightChild, v) + 1;
}
```

复制

为了要凑成 5，我们需要的值是 47（根据递归规律来找到合适的数字即可）

![img](https://wdxtub.com/images/14545241927519.jpg)

通关！撒花！

## 总结

相信通过这次『拆弹』的历练，一定对数据在内存中以及汇编的表示方法有了更加深刻的认识，做得过程可能有时候会摸不着头脑，这个时候一定要冷静，相信自己。
