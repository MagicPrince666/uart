/**
 * @file xepoll.cc
 * @author 黄李全 (hlq@ldrobot.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-10
 * @copyright Copyright (c) {2021} 深圳乐动机器人版权所有
 */

#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "xepoll.h"

Xepoll::Xepoll(void) { epfd_ = ::epoll_create(EPOLL_FD_SETSIZE); }

Xepoll::~Xepoll(void) {
  ::close(epfd_);
  std::cout << "xepoll deinit" << std::endl;
}

int Xepoll::xepoll_add(int fd) {
  ev_.data.fd = fd;
  ev_.events = EPOLLIN | EPOLLET;

  return ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev_);
}

// 添加到epoll事件，默认设置为非阻塞且fd的端口和地址都设为复用
int Xepoll::add(int fd, std::function<void()> handler) {
  listeners_[fd] = handler;
  ev_.data.fd = fd;
  ev_.events = EPOLLIN;

  // 设置为非阻塞
  int sta = ::fcntl(fd, F_GETFD, 0) | O_NONBLOCK;
  if (::fcntl(fd, F_SETFL, sta) < 0) {
    return -1;
  }

  // 地址、端口复用
  int opt = 1;
  ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  return ::epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev_);
}

int Xepoll::xepoll_del(int fd) {
  ev_.data.fd = fd;

  return ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
}

int Xepoll::del(int fd) {
  ev_.data.fd = fd;
  ::epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
  // erase: 0 不存在元素 1 存在元素
  return listeners_.erase(fd) - 1;
}

int Xepoll::loop() {
  while (1) {
    nfds_ = ::epoll_wait(epfd_, events_, MAXEVENTS, 1000);

    if (nfds_ == -1) {
      ::perror("loop");
      ::exit(1);
    }

    // if (nfds_ == 0) {
    //   std::cout << "Epoll time out" << std::endl;
    // }

    for (int i = 0; i < nfds_; i++) {
      // 有消息可读取
      if (events_[i].events & EPOLLIN) {
        // 在map中寻找对应的回调函数
        auto handle_it = listeners_.find(events_[i].data.fd);
        if (handle_it != listeners_.end()) {
          handle_it->second();
        } else {
          std::cout << "can not find the fd:" << events_[i].data.fd << std::endl;
        }
      }
    }
  }

  return 0;
}
