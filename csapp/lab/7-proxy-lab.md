[Proxy Lab - 实现一个多线程带缓存的代理服务器](http://wdxtub.com/csapp/thick-csapp-lab-7/2016/04/16/)

<!--ts-->
      * [任务目标](#任务目标)
      * [Sequential Web Proxy](#sequential-web-proxy)
      * [Multiple Concurrent Requests](#multiple-concurrent-requests)
      * [Caching Web Objects](#caching-web-objects)
      * [调试工具](#调试工具)
      * [解题思路](#解题思路)
      * [注意事项](#注意事项)
      * [总结](#总结)

<!-- Added by: anapodoton, at: Thu Feb  6 23:01:06 CST 2020 -->

<!--te-->

## 任务目标

实现一个多线程带缓存的代理服务器！

这次的作业主要分三个部分：

1. Sequential Proxy: 接收客户端发送的 HTTP 请求，解析之后向目标服务器转发，获得响应之后再转发回客户端
2. Concurrent Proxy: 在第一步的基础上，支持多线程
3. Cache Web Objects: 使用 LRU 缓存单独的对象，而不是整个页面

样例测试命令：`curl --proxy 128.2.220.15:45962 http://www.cs.cmu.edu`

## Sequential Web Proxy

第一步是实现一个简单的代理服务器，只处理 `HTTP/1.0 GET` 请求。具体步骤为

- 端口号在命令行指令中指定
  - 申请自己的端口 `$ ./port-for-user.pl dawang`，这里申请的总是偶数，所以如果需要一个额外的端口，直接在端口号 +1 即可
  - 不要随便指定端口，不然很可能干扰到别人
  - `$ ./proxy 12345`
- 监听从该端口进入的所有请求
- 解析请求，并转发合法的 HTTP 请求
  - 假设请求为 `GET http://www.cmu.edu/hub/index.html HTTP/1.1`
  - 则主机名 `www.cmu.edu`
  - 请求的页面 `/hub/index.html`
  - HTTP 请求每行以 `\r\n` 结束，以一个空行 `\r\n` 结尾
  - 需要判断地址里有没有带端口
- 把从服务器获取到的响应返回给客户端

请求的 header 也很重要，一定要有的内容是：

- `Host`: 如 `Host: www.cmu.edu`
- `User-Agent`: 如 `User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3`
- `Connection`: 必须发送 `Connection: close`
- `Proxy-Connection`: 必须发送 `Proxy-Connection: close`

## Multiple Concurrent Requests

使用 POSIX 线程，最好在线程一开始执行 `pthread_detach(pthread_self());` 这样就不用自己负责清理线程了。

注意竞争条件，尽量减少共享资源，访问共享资源的时候需要同步。

`open_clientfd` 和 `open_listenfd` 函数是线程安全的

## Caching Web Objects

具体缓存的机制是 LRU，一些具体的参数是：

- 缓存大小限制 `MAX_CACHE_SIZE = 1 MiB`，注意只缓存 web 对象，其他诸如 metadata 应该忽略
- 单个文件大小限制 `MAX_OBJECT_SIZE = 100 KiB`
- 如果有 T 个连接，那么最大的空间为 `MAX_CACHE_SIZE + T * MAX_OBJECT_SIZE`

同步问题可以参考『读者-写者问题』

## 调试工具

- Telnet: 不安全的 ssh，需要手动构造 HTTP 请求，如果想要测试非法的 header，这个功能就很有用

  - `man telnet`
  - `telnet www.wdxtub.com`
  - `GET http://www.wdxtub.com HTTP/1.0`

- cURL: 会自动构建 HTTP 请求

  - `curl http://www.wdxtub.com`
  - 代理模式 `curl --proxy lemonshark.ics.cs.cmu.edu:3092 http://www.wdxtub.com`

- ```
  netcat
  ```

  : 多用途网络工具，用法与

   

  ```
  telnet
  ```

   

  类似

  - `nc catshark.ics.cs.cmu.edu 12345`
  - `GET http://www.cmu.edu/hub/index.html HTTP/1.0`

## 解题思路

代理服务器作为客户端和服务器的中间人，既需要我们了解客户端的知识，也需要懂得服务器的知识。整个过程如下：

- 客户端发送请求给代理服务器
- 代理服务器转发请求给实际访问的服务器
- 实际访问的服务器返回响应给代理服务器
- 代理服务器转发响应给客户端

这个实验思路不难，难在具体的实现，和对 HTTP 的基本认识。

## 注意事项

- 大端小端
- 能够处理各种 URL（合法或非法）
- 不是所有的内容都是 ASCII 码，注意选择对应的函数来处理二进制文件（图像和视频）
- 所有的请求都用 `HTTP/1.0` 来转发
- 需要处理 `SIGPIPE` 信号，默认的操作是关闭进程，这里应该屏蔽这个信号
- 使用 Robust I/O package 的 `read`, `write`, `fread`, `fwrite` 来增加健壮性
- 如果调用 `read` 来获取已经被关闭的 socket，会返回 -1，并给出 `ECONNRESET` 错误，不应该因为这个错误而导致进程终结
- 如果调用 `write` 来获取已经被关闭的 socket，会返回 -1，并给出 `EPIPE` 错误，不应该因为这个错误而导致进程终结
- 代码注意模块化
- 因为可以写单独的文件，需要对应更新 Makefile
- Header 注意格式规范
- 做 Cache 的时候注意指针

其他需要注意的地方（来自[这里](http://lifeofzjs.com/blog/2014/02/21/csapp-proxy-lab/)）

在 proxy 打开与 server 的 TCP 连接的时候，需要调用 `gethostbyname` 或者`gethostbyaddr` 来通过 DNS 获取 server 主机的DNS信息，比如 ip 地址，别名之类的，返回的是一个 struct 的指针。但是这个 struct 是一个静态变量，也就是说这些函数不支持多线程的访问，是线程不安全的。解决方法是定义一个 mutex 来加锁，任意时刻只能又一个线程在调这些函数。

调用 `Signal(SIGPIPE, SIG_IGN);` 将 `SIGPIPE` 这个信号忽略掉。如果尝试两次发送数据到一个已经被对方关闭的 socket 上时，内核会发送一个 SIGPIPE 信号给程序，在默认情况下，会终止当前程序，显然不是我们想要的，所以要忽略它。这里有一个 stackoverflow 上的[相关问题](http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly)。还有一点，往 broken pipe 里写会使 errno 等于 EPIPE，而往 broken pipe 里读会使 errno 等于 ECONNRESET。

HTTP/1.1 里默认将 connection 定义为 keep-alive，也就是一条 TCP 连接可以处理多个请求，不用每次都要重新建立 TCP 连接。我们的简易 proxy 还无法提供这样的功能，所以在读 client 发过来的 header 的时候，如果是 `Connection: keep-alive` 或者 `Proxy-Connection: keep-alive`，我们都要把它们换成 `Connection: close` 或 `Proxy-Connection: close`。

创建线程以后记得要detach掉，否则这个线程结束后不会释放资源直到有别的线程join了这个线程。

如果 header 里没有 `Content-Length` 这一项，怎么确定 body 的长度？这个问题一直没想过直到现在遇到了这个问题。这个长度写到了 body 里，这种方式叫做 Transfer Encoding。因为服务器在处理静态对象时，事先知道对象的大小；而在处理动态对象时，无法事先知道 body 的长度。实现的时候分两种情况来从 socket 中读数据。

需要正确关闭所有的文件描述符。系统给一个程序能打开的描述符数量做了一个限制。如果是 ubuntu 下，可以通过 `cat /proc/sys/fs/file-max` 来查看最大文件描述符数。在 proxy 运行一段时间，确保描述符不会持续增加。在 ubuntu 下查看程序打开的描述符方法：找到程序的pid，然后 `cat /proc/$pid/fd`。

## 总结

至此，读厚/读薄 CSAPP 系列终于告一段落，这应该是我完成的第一个技术系列。想说的话很多，从中学到的东西更多。看到自己的文章对大家有那么一点帮助，深感哪怕是一点微小的工作，也应该做下去，更幸运的是因此认识了许许多多朋友，只有自己做出有价值的东西，别人才会真正认同吧。

人生在世，白驹过隙，能做的事情并不算多。如果不想碌碌无为一辈子，那么就要找到自己的方向，然后坚定地走下去。歌女的歌，舞者的舞，剑客的剑，文人的笔，英雄的斗志，都是这样子的，只要是不死，就不能放弃。

与诸君共勉。
