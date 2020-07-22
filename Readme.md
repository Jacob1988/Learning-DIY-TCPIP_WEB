# 项目简介
本项目是配套视频课程《自己动手写TCP+WEB服务器》的配套源码。

该项目从0行代码开始，逐行编写至大概**1500行**（不含注释），从而实现一个非常简单的TCP/IP协议栈，并在此基础上实现了一个简单的WEB服务器。

由于代码量极少，所以很容易理解。当然具体实现上会有非常多的功能裁剪，例如不支持IP分片发送及重组，TCP仅支持序列号同步。

通过简单的实现，可帮你快速理解TCP/IP最核心的工作原理。不必在不必要的细节上纠缠。

# 视频课程是否免费？
**是的。完全免费。**
如需访问视频课程，请访问:[http://01ketang.cc](http://01ktang.cc/) 获取相关地址.

# 联系我
网站：http://01ketang.cc/about
课程：http://01ketang.cc/xnet-tiny
邮箱：527676163#qq.com (#换成@)

# 开发工具
* visual stduio 2019(社区、专业版都可)
* npcap

下载后，请安装npcap, 否则**编译成功后却无法运行**。该软件包，请访问课程网站获取。

# 章节列表
* 第0章 基本工程的测试
    * c0000 基本工程编译测试
* 第1章 以太网数据包收发实现
    * c0100 网络数据包定义及接口实现
    * c0101 以太网的数据收发驱动封装
    * c0102 以太网输入输出处理的实现
* 第2章 ARP协议的实现
    * c0200 ARP初始化
    * c0201 无回报ARP的生成
    * c0202 ARP输入处理
    * C0203 ARP的超时重新请求
* 第3章 IP协议的实现
    * c0300 实现IP层的输入处理
    * c0301 实现IP层的输出处理
* 第4章 ICMP协议的实现
    * c0400 实现ping的响应
    * c0401 实现目标不可达的响应
* 第5章 UDP协议及时间服务器的实现
    * c0500 建立UDP结构
    * c0501 添加UDP基本操作接口
    * c0502 时间服务器的接收处理
    * c0503 UDP输入处理
    * c0504 UDP输出处理
* 第6章 TCP协议的实现
    * c0600 建立TCP结构 
    * c0601 添加TCP基本操作接口
    * c0602 基本TCP输入处理
    * c0603 TCP的三次握手
    * c0604 TCP的连接断开
    * c0605 TCP的数据发送
    * c0606 TCP的数据接收
* 第7章 HTTP服务器的实现
    * c0700 网页结构与设计简介
    * c0701 缓存所有HTTP连接
    * c0702 实现网页的返回
    * c0703 返回404和400错误

