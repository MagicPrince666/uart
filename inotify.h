/**
 * @file inotify.h
 * @author 黄李全
 * @brief 
 * @version 0.1
 * @date 2021-06-10
 * @copyright 
 */
#pragma once

#include "xepoll.h"
#include "interface.h"

class Inotify {
 public:
  Inotify(Xepoll *epoll, Interface *interface, const std::string name);
  ~Inotify();

  int handle_event();

 private:
    Xepoll *epoll_;
    int fd;
    int wd;
    int len;
    int nread;
    char buf[BUFSIZ];
    struct inotify_event *event;
    Interface *m_interface_;
    std::string file_name_;
    std::string ReadFileIntoString(const std::string& path);
};
