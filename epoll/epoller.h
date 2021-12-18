/**
 * @file epoller.h
 * @author 黄李全 (hlq@ldrobot.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-26
 * @copyright Copyright (c) {2021} 深圳乐动机器人版权所有
 */

/** Todo
 * - [ ] Continuous integration
 *   - [ ] Linux
 *   - [ ] Windows
 *   - [ ] Mac
 * - [ ] Add support for kqueue
 * - [ ] Add support for windows equivalent
 */
#pragma once

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdlib.h>
#include <iostream>
#include <unordered_map>

#define MAX_PENDING 1024
#define BUFFER_SIZE 1024

class Handler {
public:
	virtual ~Handler() {}
	virtual int handle(epoll_event e) = 0;
};

/**
 * epoll 事件轮询
 */ 
class IOLoop {
public:
	static IOLoop *Instance()
	{
		static IOLoop instance;
		return &instance;
	}
	~IOLoop() 
	{
		for (auto it : handlers_) {
			delete it.second;
		}
	}

	void start()
	{
		const uint32_t MAX_EVENTS = 10;
		epoll_event events[MAX_EVENTS];
		while (true)
		{
			// -1 只没有事件一直阻塞
			int nfds = epoll_wait(epfd_, events, MAX_EVENTS, 1000/*Timeout*/);
			if(nfds < 0) {
				std::cout << "epoll failure" << std::endl;
				break;
			} else if(nfds == 0) {
				std::cout << "epoll time out" << std::endl;
				continue;
			}

			for (int i = 0; i < nfds; ++i) {
				if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ) // error
				{
					std::cerr << "[E] epoll event error fd = " << events[i].data.fd << std::endl ;
					removeHandler(events[i].data.fd);
					close(events[i].data.fd);
					exit(1);
				} else {
					int fd = events[i].data.fd;
					Handler* handler = handlers_[fd];
					handler->handle(events[i]);
				}
			}
		}
	}

	void addHandler(int fd, Handler* handler, unsigned int events)
	{
		handlers_[fd] = handler;
		epoll_event e;
		e.data.fd = fd;
		e.events = events;

		std::cout << "add handle, fd=" << fd << std::endl;
		if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &e) < 0) {
			std::cout << "Failed to insert handler to epoll" << std::endl;
		}
	}

	void modifyHandler(int fd, unsigned int events) 
	{
		struct epoll_event event;
		event.events = events;
		epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
	}

	void removeHandler(int fd) 
	{
		Handler* handler = handlers_[fd];
		handlers_.erase(fd);
		delete handler;
		//将fd从epoll堆删除
		epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
	}

private:
	IOLoop()
	{
		epfd_ = epoll_create1(0);  //flag=0 等价于epll_craete
		if (epfd_ < 0) {
			std::cout << "Failed to create epoll" << std::endl;
			exit(1);
		}
	}

private:
	int epfd_;
	std::unordered_map<int, Handler*> handlers_;
};

class UdsHandler : public Handler {
public:
	UdsHandler(const char *sock_path);
	~UdsHandler();
	virtual int handle(epoll_event e) override;

private:
	struct sockaddr_un addr;
	socklen_t addrlen;
	int sock_fd = -1;
	void setnonblocking(int fd);
	char *buffer = NULL;
};

