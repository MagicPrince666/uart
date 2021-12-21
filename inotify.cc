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
    fd = inotify_init();
    if (fd < 0) {
        fprintf(stderr, "inotify_init failed\n");
        exit(1);
    }

    //保证文件存在
    if (-1 == access( name.c_str(), F_OK )) {
        std::ofstream file;
        file.open(name.c_str());
        file.close();
    }

    wd = inotify_add_watch(fd, name.c_str(), IN_ALL_EVENTS);
    if (wd < 0) {
        fprintf(stderr, "inotify_add_watch %s failed\n", name.c_str());
        exit(1);
    } else epoll_->add(fd, std::bind(&Inotify::handle_event, this));

}

Inotify::~Inotify()
{
    close(fd);
    std::cout << "inotify deinit" << std::endl;
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
    if((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
        nread = 0;
        while (len > 0) {
            event = (struct inotify_event *)&buf[nread];
            for (int i = 0; i < EVENT_NUM; i++) {
                if ((event->mask >> i) & 1) {
                    if (event->len > 0) {
                        fprintf(stdout, "%s --- %s\n", event->name, event_str[i]);
                    } else {
                        if(i == 3) {
                            // fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
                            std::string get_cmd = ReadFileIntoString(file_name_);
                            if(nullptr != m_interface_) {
                                m_interface_->Transfer(get_cmd);
                            }
                        }
                    }
                }
            }
            nread = nread + sizeof(struct inotify_event) + event->len;
            len = len - sizeof(struct inotify_event) - event->len;
        }
    }
    return 0;
}
