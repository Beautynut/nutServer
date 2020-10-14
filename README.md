# A C++ High Performance Server

## Introduction

> 这是一个我个人在学习muduo之后,基于muduo做的多线程服务器  
HTTP之前的部分(也就是从EventLoop到TcpServer),很大部分代码都是参考muduo的,因为自己写着写着总觉得不好,然后看了Muduo的代码发现每个地方都比自己写的好 (还挺不好意思的-:)) 

> 目前处于大量有关http的bug修改中

## Version
> V1.0 TCP Server部分完成
> V1.1 支持epoll

## Technical  Points

> + 使用线程池避免线程频繁创建销毁的开销
> + 主线程只负责accept请求,然后分发给其他io线程,锁的争用只出现在主线程和某一io线程之间,io线程之间不产生锁的争用.
> + 使用eventfd实现了线程的异步唤醒
> + 使用双缓冲区技术实现了简单的异步日志系统
> + 为减少内存泄漏的可能，使用智能指针等RAII机制

## TODO

> 1.支持epoll
> 2.支持http
> 3.支持定时器关闭超时连接
> 4.支持POST GET
> 5.支持日志级别设置
