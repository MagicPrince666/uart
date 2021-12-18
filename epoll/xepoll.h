/**
 * @file xepoll.h
 * @author 黄李全 (hlq@ldrobot.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-10
 * @copyright Copyright (c) {2021} 深圳乐动机器人版权所有
 */

#ifndef __XEPOLL_H__
#define __XEPOLL_H__

#include <sys/socket.h>
#include <sys/epoll.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#define UDP_BUF_SIZE 1472
#define TCPBUF_SIZE 1460
#define MAXEVENTS 20
#define EPOLL_FD_SETSIZE    1024

class Xepoll {
 public:
  Xepoll();
  ~Xepoll();
  int xepoll_add(int fd);
  int add(int fd, std::function<void()> handler);
  int xepoll_del(int fd);
  int del(int fd);
  int loop();

 private:
  struct epoll_event ev_, events_[MAXEVENTS];
  int epfd_;
  int nfds_;
  // map<fd, cllback<int(buf, udp_cli_t)>>
  std::unordered_map<int, std::function<void()>> listeners_;
};

#endif
