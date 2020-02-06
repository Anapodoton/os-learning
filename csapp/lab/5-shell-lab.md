[Shell Lab - 实现一个 shell](http://wdxtub.com/csapp/thick-csapp-lab-5/2016/04/16/)

<!--ts-->
      * [任务目标](#任务目标)
      * [Shell 简介](#shell-简介)
      * [提示](#提示)
      * [解题攻略](#解题攻略)
      * [附录: 中文 man 文档](#附录-中文-man-文档)
      * [总结](#总结)
      * [参考资料](#参考资料)

<!-- Added by: anapodoton, at: Thu Feb  6 23:00:53 CST 2020 -->

<!--te-->

## 任务目标

这次的实验，我们需要自己完成一个简单的 shell 程序，通过具体的实现，我们可以更加深入地计算机运行的机制（尤其是 Exceptional Control Flow 和进程）。

在具体开始之前，最好先复习一下基本概念。注意！这次任务不简单！但是老师提供了很多辅助函数，确定先读懂已有代码再开始（注意代码风格），需要仔细查看的 man pages：

- `sigemptyset()`
- `sigaddset()`
- `sigprocmask()`
- `sigsuspend()`
- `waitpid()`
- `open()`
- `dup2()`
- `setpgid()`
- `kill()`

我们具体要做的是在 `tsh.c` 中补全下列空函数，这个文件中已经包含了一个基本的 shell 程序，但是还有很多东西没有完成：

- `void eval(char *cmdline)`：解析命令与执行，约 300 行
- `void sigchld_handler(int sig)`：捕获 SIGCHLD 信号
- `void sigtstp_handler(int sig)`：捕获 SIGTSTP(ctrl-z) 信号
- `void sigint_handler(int sig)`：捕获 SIGINT(ctrl-c) 信号

具体的实现规格：

- 每一行会输出一个 `tsh>`，然后等待用户输入

- 用户的输入包括 `name` 加上零个或多个参数，这些参数之间用一个或多个空格分隔。如果 `name` 是内置命令，那么直接执行，否则需要新建一个子进程，并在子进程中完成具体的工作

- 不需要支持管道，但是需要支持输入输出重定向，如

   

  ```
  tsh> /bin/cat < foo > bar
  ```

  （必须支持在同一行重定向输入以及输出）

  - 也需要支持内置命令的重定向，如 `tsh> jobs > foo`

- 输入 `ctrl-c` 或 `ctrl-z` 会给当前的前台进程（包括其子进程）发送 SIGINT(SIGTSTP) 信号，如果没有前台任务，那么这俩信号没有任何效果

- 如果输入的命令以 `&` 结尾，那么就要以后台任务的方式执行，否则按照前台执行

- 每个 job 都有其进程 ID(PID) 和 job ID(JID)，都是由 tsh 指定的正整数，JID 以 `%` 开头（如 `%5` 表示 JID 为 5，而 `5` 则表示 PID 为 5），这部分已提供了辅助函数

- 支持的内置命令有

  - `quit` 退出 shell
  - `jobs` 列出所有的后台任务
  - `bg job` 给后台 `job` 发送 SIGCONT 信号来继续执行该任务，具体的 `job` 数值可以是 PID 或 JID
  - `fg job` 给前台 `job` 发送 SIGCONT 信号来继续执行该任务，具体的 `job` 数值可以是 PID 或 JID

- tsh 应该回收所有的僵尸进程，如果任何 job 因为接收了没有 catch 的信号而终止，tsh 应该识别出这个时间并且打印出 JID 和相关信号的信息

测试的时候先 `make` 然后 `./tsh` 即可，不过一开始好像没办法退出。如果需要参考，可以试试 `tshref` 这个程序。确定无误之后可以进行完整测试。

这里我们用 trace 文件来测试，具体使用命令 `./runtrace` 来测试，具体用法如下

```
# 查看帮助
./runtrace -h

# 测试某个特性
./runtrace -f trace05.txt -s ./tsh
```

复制

如果想要进行完整的测试，可以使用 `./sdriver`，具体用法如下

```
# 查看帮助
./sdriver -h

# 一般来说可以直接使用默认设置测试
./sdriver
```

复制

系统会自动评分，具体每个文件在测试的内容是：

![trace 文件内容](https://wdxtub.com/images/14575490108688.jpg)

## Shell 简介

简单来说，shell 有两种执行模式：

1. 如果用户输入的命令是内置命令，那么 shell 会直接在当前进程执行（例如 `jobs`）
2. 如果用户输入的是一个可执行程序的路径，那么 shell 会 fork 出一个新进程，并且在这个子进程中执行该程序（例如 `/bin/ls -l -d`）

第二种情况中，每个子进程称为一个 job（当然也可以不止一个，通过管道机制，不过我们这里的实现不需要考虑管道）

如果命令以 `&` 结束，那么这个 job 会在后台执行（比如 `/bin/ls -l -d &`），也就是说 shell 本身不会等待 job 执行完成，直接可以继续输入其他命令；而在其他情况下，则是在前台运行，shell 会等待 job 完成，用户才可以继续输入命令。也就是说同一个时间只可能有一个前台任务，但是后台任务可以有任意多个。

程序的入口是 `int main(int argc, char *argv[])`，对于 `/bin/ls -l -d` 来说，我们有：

```
argc == 3
argv[0] == ''/bin/ls''
argv[1] == ''-l''
argv[2] == ''-d''
```

复制

另外两个需要支持功能是：

- job control：允许用户更改进程的前台/后台状态以及京城的状态(running, stopped, or terminated)
  - ctrl-c 会触发 SIGINT 信号并发送给每个前台进程，默认的动作是终止该进程
  - ctrl-z 会触发 SIGTSTP 信号并发送给每个前台进程，默认的动作是挂起该进程，直到再收到 SIGCONT 信号才继续
  - `jobs` 命令会列出正在执行和被挂起的后台任务
  - `bg job` 命令可以让一个被挂起的后台任务继续执行
  - `fg job` 命令可以让一个被挂起的前台任务继续执行
- I/O redirection：重定向输入输出
  - `tsh> /bin/ls > foo`
  - `tsh> /bin/cat < foo`

## 提示

- 不要使用 `sleep()` 来同步
- 不要使用忙等待 `while(1);`
- 使用 `sigsuspend` 来同步
- 竞争条件
- 僵尸进程回收（注意竞争条件以及正确处理信号）
- 等待前台任务（仔细思考怎么样才是好的方式）
- 不要假定进程的执行顺序
- 子进程挂掉的时候应该在一个限定时间内被回收
- 不要在多个地方调用 `waitpid`，很容易造成竞争条件，也会造成程序过分复杂
- 不要使用任何系统调用来管理 terminal group
- `waitpid`, `kill`, `fork`, `execve`, `setpgid`, `sigprocmask` 和 `sigsuspend` 都非常有用，`waitpid` 中的 WUNTRACED 和 WNOHANG 选项也是如此。
- 遇到不清晰的用 `man` 来查看细节
- 实现 signal handler 的时候注意给全部的前台进程组发送 SIGINT 和 SIGTSTP 信号
- 在 `kill` 函数中使用 `-pid` 的格式作为参数
- 在 shell 等待前台工作完成时，需要决定在 `eval` 及 `sigchold handler` 具体的分配，这里有一定技巧
- 在函数 `eval` 中，在 fork 出子进程之前，必须使用 `sigprocmask` 来阻塞 SIGCHLD, SIGINT 和 SIGTSTP 信号，完成之后再取消阻塞。调用 `addjob` 的时候也需要如此。注意，因为子进程也继承了之前的各种状态，所以在子进程中调用 `exec` 执行新程序的时候注意需要取消阻塞，同样也需要恢复默认的 handler（shell 本身已经忽略了这些信号），具体可以看书本的 8.5.6 节
- 不要使用 `top`, `less`, `vi`, `emacs` 之类的复杂程序，使用简单的文本程序如：`/bin/cat`, `/bin/ls`, `/bin/ps`, `/bin/echo`
- 因为毕竟不是真正的 shell，所以在 fork 之后，execve 之前，子进程需要调用 `setpgid(0, 0)`，这样就把子进程放到一个新的进程组里。这样就保证我们的 shell 前台进程组中唯一的进程，当按下 ctrl-c 时，应该捕获 SIGINT 信号并发送给对应的前台进程组中。

同样提供一个 `tshref` 参考程序来作为比对输出（除了进程 id 之外其他需要一模一样），具体是通过 `runtrace` 文件来测试，每个 trace 文件会测试一个特性

## 解题攻略

最开始当时是要先读懂代码，尤其是整个程序到底在干什么，如果有仔细看我前面的介绍和课本的话，应该比较轻松能找到对应（毕竟这是一个简化的版本），所以这里废话不多说，直接开始完成基础工作。我们先来看看如何改动 `eval` 这个函数。

在这个函数中，我们会先解析命令（具体的解析已经有工具函数），然后得到一系列 token，结构如下

![token 的结构](https://wdxtub.com/images/14575497659305.jpg)

目前来说，对我们最有用的是这里面的枚举类型，我们可以先用这个来判断是否是内置函数，据此来决定走哪条分支。我们先把最基本的退出功能做了，这样就不会出现一旦开始就没办法结束的情况，具体方法也很简单，直接 `exit(0);` 即可，我们测试一下，发现已经可以正确退出了：

![退出程序](https://wdxtub.com/images/14575502436640.jpg)

接着我们来实现 `jobs` 这个命令，因为已经提供了 `listjobs` 这个函数，所以我们直接围绕着这个函数来做文章即可。留意到 token 结构体中有 `infile` 和 `outfile` 两项，这个就是用来重定向的判断（我们不需要担心解析的问题，可以直接用）。同样，我们来判断一下有没有 `outfile`，对应进行处理即可。注意输出的时候如果不需要重定向，那么就输出到 stdout，如果需要重定向，就输出到对应的 file descriptor 中（打开文件的时候需要设定 flag，具体可以 `man open` 进行查看）。

接着我们来实现 FG 和 BG 这两个命令，我们需要注意的地方有两个，一个是先根据判断传入的是 JID 还是 PID，然后发送信号之后需要等待进程完成（这里注意使用 `sigsuspend`）。

这里需要注意 `.` 和 `->` 这两个操作符的不同，简单来说，就是如果左边是一个指向结构体的指针，那么就要用 `->`；如果是一个结构体，那么就要用 `.`。

这些做完之后我们可以先来简单测试一下

```
./runtrace -f trace00.txt -s ./tsh
./runtrace -f trace00.txt -s ./tshref
./runtrace -f trace01.txt -s ./tsh
./runtrace -f trace01.txt -s ./tshref
```

复制

![测试结果](https://wdxtub.com/images/14575550290784.jpg)

看到和参考程序输出至少是一致的，我们就可以继续了。

接着我们需要来处理非内置命令的情况，参考课件中的代码，先把需要用到的 mask 之类的弄好，并且我们暂时不考虑重定向的问题。然后需要把对应的 signal handler 补充完整。

这之后我们可以来跑一次测试 `./sdriver`，最后的得分是 60/100，第一个错误出现在 trace08.txt 这个文件中，查表得知是发送 fatal SIGINT 给前台进程。我自己用 `./runtrace -f trace08.txt -s ./tsh` 测试了几次，发现有时候可以正确输出，有时候则会超时，说明是处理进程同步的时候出了问题。经过检查发现，阻塞信号的时候需要阻塞全部信号（之前我只阻塞了 SIGCHLD 会出问题），再测试的话，发现已经有了 80/100 分。

继续看看哪里出了问题，在输出的日志中寻找最先出错的地方，发现是 trace22.txt，功能是 I/O redirection(input)，这就对了，毕竟我们还没写这个功能。

我们现在就来写一下。不过在此之前，回答一下前面的问题，前面提到过：

> 在 shell 等待前台工作完成时，需要决定在 `eval` 及 `sigchold handler` 具体的分配，这里有一定技巧

那么技巧是什么呢？其实很简单，就是都在 eval 里做，handler 尽量短小精悍。

好，我们继续来做输入输出重定向，同样分为内置函数与其他两个类型，内置函数唯一需要输出的是 `jobs` 这个函数，不过我们之前已经处理过，这里暂且不管（出问题再说）。所以把主要精力集中在非内置的函数上。具体应该在 `setpgid(0,0)` 这句之后，且应该在 `execve` 之前。具体的操作也比较简单，就是打开文件（只读），然后利用 `dups` 重定向到 STDIN 中即可。对于输出的情况也是类似的，这里不赘述。唯一需要注意的一点是打开文件时候的 flag，设置错误会导致没办法正确重定向。

改完错误之后发现 trace15.txt 又出错了，而且经过测试发现死锁的问题还在，而且是内置函数的问题（果然一开始有小问题），后来发现是搞错了一个变量（但是仍旧有小概率会出现死锁，不过提交的时候似乎一切正常）

最后需要注意的是有些测试会直接修改源代码，所以每次都需要重新解压（还是蛮讨厌的）。总体来说只要理解了整个过程就不算太难，使用 `csapp.h` 的时候可能需要把代码复制到 `tsh.c` 中。

## 附录: 中文 man 文档

如果觉得看英文太累（虽然建议看英文），可以使用中文的 man 文档，具体的使用步骤如下：

在[这里](http://manpages-zh.googlecode.com/files/manpages-zh-1.5.1.tar.gz)下载安装包，然后通过如下命令进行安装：

```
tar zxvf manpages-zh-1.5.1.tar.gz
cd manpages-zh-1.5.1
./configure --prefix=/usr/local/zhman --disable-zhtw
make && make install
```

复制

在 Mac 上会乱码，所以需要安装 groff，具体命令为：

```
brew install homebrew/dupes/groff
```

复制

然后打开 `/etc/man.conf`，把 `NROFF` 的那一行改为：

```
NROFF preconv -e UTF8 | /usr/local/bin/nroff -Tutf8 -mandoc -c
```

复制

最后我么加一个别名，方便使用（根据自己使用的 shell 来针对改，bash 的话是 ~/.bashrc，zsh 的话是 ~/.zshrc），在文件中加入这么一句：

```
alias cman='man -M /usr/local/zhman/share/man/zh_CN'
```

复制

然后 `source .zshrc` 启用，我们就可以通过 `cman` 命令来查看了，比如说输入 `cman kill`，就可以看到

![效果](https://wdxtub.com/images/14575529137049.jpg)

另外推荐一个比 man 更好用的文档，叫做 [TLDR](http://www.ostera.io/tldr.jsx/#/?_k=9t2oqt)，具体效果如下

![img](https://wdxtub.com/images/14620371208361.jpg)

直接给出最常用的用法，不用自己搜索了，安装之后在终端里也可以直接使用，如下图所示

![img](https://wdxtub.com/images/14620371795813.jpg)

## 总结

这一次我们实现了一个简单的 shell，相信对于这个我们平时都在用的『老朋友』有了更多的理解，更重要的是，在 shell 背后所隐含的进程线程异常控制流等等都需要彻底搞明白，这个过程可能会比较烦躁，但是一通百通，这次实验是很好的练习。

## 参考资料

- [Mac/Linux 安装man命令的中文帮助文档](https://www.yurendu.com/read/install-man-command-chinese-help-documentation-on-mac-and-linux.html)
- [TLDR](http://tldr-pages.github.io/#cli-installation)
