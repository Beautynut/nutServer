# A C++ High Performance Server

## Introduction

> 这是一个我个人在学习muduo之后,基于muduo做的多线程服务器  
HTTP之前的部分(也就是从EventLoop到TcpServer),很大部分代码都是参考muduo的,因为自己写着写着总觉得不好,然后看了Muduo的代码发现每个地方都比自己写的好 (还挺不好意思的-:)) 

> 目前仍有许多不足之处 请多多包涵

## Usage
> cmake [path]  
> make  
> ./test1  
> 我监听的是9981端口,信息会直接写到日志里,目前有一个bug是日志缓冲区没有写满之前会一直阻塞,直到缓冲区写满再写入日志,所以需要多发几次请求...(等搬完家再改这个Bug吧...)

## Version
> V1.0 TCP Server部分完成  
> V1.1 支持epoll  
> V2.0 支持Http

## Technical  Points

> + 线程模型使用one loop per thread  
> + IO方面则使用epoll水平触发,非阻塞IO,Reactor模式
> + 使用线程池避免线程频繁创建销毁的开销
> + 主线程只负责accept请求,然后分发给其他io线程,锁的争用只出现在主线程和某一io线程之间,io线程之间不产生锁的争用.
> + 使用eventfd实现了线程的异步唤醒
> + 使用双缓冲区技术实现了简单的异步日志系统
> + 为减少内存泄漏的可能，使用智能指针等RAII机制
> + 使用状态机解析Http请求

## TODO

> 1.支持定时器关闭超时连接
> 2.支持POST GET
> 3.支持日志级别设置  

## Languages
| language | files | code | comment | blank | total |
| :--- | ---: | ---: | ---: | ---: | ---: |
| C++ | 47 | 3,070 | 113 | 581 | 3,764 |
| Markdown | 1 | 18 | 0 | 8 | 26 |
