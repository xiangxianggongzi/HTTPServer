#include "Poller.h"
#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h>

#define EVENTNUM 4096 //最大触发事件数量
#define TIMEOUT 1000 //epoll_wait 超时时间设置

Poller::Poller()
    : pollfd_(-1),
    eventlist_(EVENTNUM),
    channelmap_(),
    mutex_()
{
    pollfd_ = epoll_create(256);
    if(pollfd_ == -1)
    {
        perror("epoll_create1");
        exit(1);
    }
    std::cout << "epoll_create" << pollfd_ << std::endl;
}

Poller::~Poller()
{
    close(pollfd_);
}

//等待I/O事件
void Poller::poll(ChannelList &activechannellist)
{
    int timeout = TIMEOUT;
    int nfds = epoll_wait(pollfd_, &*eventlist_.begin(), (int)eventlist_.capacity(), timeout);
    if(nfds == -1)
    {
        perror("epoll wait error");
    }
    for(int i = 0; i < nfds; ++i)
    {
        int events = eventlist_[i].events;
        Channel *pchannel = (Channel*)eventlist_[i].data.ptr;
        int fd = pchannel->GetFd();

        std::map<int, Channel*>::const_iterator iter;
        {
            std::lock_guard <std::mutex> lock(mutex_);
            iter = channelmap_.find(fd);
        }        
        if(iter != channelmap_.end())
        {
            pchannel->SetEvents(events);
            activechannellist.push_back(pchannel);
        }
        else
        {            
            std::cout << "not find channel!" << std::endl;
        }
    }
    if(nfds == (int)eventlist_.capacity())
    {
        std::cout << "resize:" << nfds << std::endl;
        eventlist_.resize(nfds * 2);
    }
}

//添加事件
void Poller::AddChannel(Channel *pchannel)
{
    int fd = pchannel->GetFd();
    struct epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        channelmap_[fd] = pchannel;
    }      

    if(epoll_ctl(pollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        perror("epoll add error");
        exit(1);
    }
}

//删除事件
void Poller::RemoveChannel(Channel *pchannel)
{
    int fd = pchannel->GetFd();
    struct epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        channelmap_.erase(fd);
    }    

    if(epoll_ctl(pollfd_, EPOLL_CTL_DEL, fd, &ev) == -1)
    {
        perror("epoll del error");
        exit(1);
    }
}

//更新事件
void Poller::UpdateChannel(Channel *pchannel)
{
    int fd = pchannel->GetFd();
    struct epoll_event ev;
    ev.events = pchannel->GetEvents();
    ev.data.ptr = pchannel;

    if(epoll_ctl(pollfd_, EPOLL_CTL_MOD, fd, &ev) == -1)
    {
        perror("epoll update error");
        exit(1);
    }
}
