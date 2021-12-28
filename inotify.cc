/**
 * @file inotify.cc
 * @author 黄李全
 * @brief 
 * @version 0.1
 * @date 2021-06-10
 * @copyright
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/stat.h>

#include <string>
#include <fstream>
#include <iostream>

#include "inotify.h"

#define EVENT_NUM 12

const char *event_str[EVENT_NUM] =
{
    "IN_ACCESS",
    "IN_MODIFY",
    "IN_ATTRIB",
    "IN_CLOSE_WRITE",
    "IN_CLOSE_NOWRITE",
    "IN_OPEN",
    "IN_MOVED_FROM",
    "IN_MOVED_TO",
    "IN_CREATE",
    "IN_DELETE",
    "IN_DELETE_SELF",
    "IN_MOVE_SELF"
};

Inotify::Inotify(Xepoll *epoll, Interface *interface, const std::string name)
: epoll_(epoll), m_interface_(interface), file_name_(name)
{
    inotify_fd_ = inotify_init();
    if (inotify_fd_ < 0) {
        fprintf(stderr, "inotify_init failed\n");
        exit(1);
    }

    //保证文件存在
    if (-1 == access( name.c_str(), F_OK )) {
        std::ofstream file;
        file.open(name.c_str());
        file.close();
    }

    int wd = inotify_add_watch(inotify_fd_, name.c_str(), IN_ALL_EVENTS);
    if (wd < 0) {
        fprintf(stderr, "inotify_add_watch %s failed\n", name.c_str());
        exit(1);
    } else epoll_->add(inotify_fd_, std::bind(&Inotify::handle_event, this));

}

Inotify::~Inotify()
{
    if(inotify_fd_ > 0) {
        close(inotify_fd_);
    }
}

std::string Inotify::ReadFileIntoString(const std::string& path) {
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}

int Inotify::handle_event()
{
    int len;
    int nread = 0;
    char buf[BUFSIZ];
    struct inotify_event *event;
    if((len = read(inotify_fd_, buf, sizeof(buf) - 1)) > 0) {
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];

            if(event->mask & IN_CLOSE_WRITE) { //关闭并写入，视为成功输入一条命令
                std::string get_cmd = ReadFileIntoString(file_name_);
                if(nullptr != m_interface_) {
                    m_interface_->Transfer(get_cmd);
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }
    return len;
}
