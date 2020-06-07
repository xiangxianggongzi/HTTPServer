//Channel类，表示每一个客户端连接的通道，对fd和相应事件的封装。

// EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
// EPOLLOUT：表示对应的文件描述符可以写；
// EPOLLHUP：表示对应的文件描述符被挂断；

#include "Channel.h"
#include <iostream>
#include <sys/epoll.h>

Channel::Channel():fd_(-1)
{}

Channel::~Channel()
{}

void Channel::HandleEvent()
{
    if(events_ & EPOLLRDHUP)//对方异常关闭
    {
        std::cout << "Event EPOLLRDHUP" << std::endl;
        closehandler_();
    }
    else if(events_ & (EPOLLIN))//读事件
    {
        std::cout << "Event EPOLLIN" << std::endl;
        readhandler_();
    }
    else if(events_ & EPOLLOUT)//写事件
    {
        std::cout << "Event EPOLLOUT" << std::endl;
        writehandler_();
    }
    else
    {
        std::cout << "Event error" << std::endl;
        errorhandler_();//连接错误
    }
}
