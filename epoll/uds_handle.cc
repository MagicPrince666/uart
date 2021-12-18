/**
 * @file test.cc
 * @author 黄李全 (hlq@ldrobot.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-26
 * @copyright Copyright (c) {2021} 深圳乐动机器人版权所有
 */

#include "epoller.h"

#define MAX_CONNECT_NUM 5

UdsHandler::UdsHandler(const char *sock_path)
{
	buffer = new char[BUFFER_SIZE];
	unlink(sock_path);
	memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock_path);
	addrlen = sizeof(addr);

	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(sock_fd < 0) {
		std::cout << "Create uds fail" << std::endl;
	} else std::cout << "Create uds socket, fd=" << sock_fd << std::endl;

	int opt = 1;
  	::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  	::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

	if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        /* handle exception */
        perror("Bind error.");
        exit(1);
    }

	setnonblocking(sock_fd);

	IOLoop::Instance()->addHandler(sock_fd, this, EPOLLIN);
}

UdsHandler::~UdsHandler()
{
	std::cout << "Quit uds" << std::endl;
	delete[] buffer;
	close(sock_fd);
}

void UdsHandler::setnonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int UdsHandler::handle(epoll_event e)
{
	int fd = e.data.fd;

	if (e.events & EPOLLHUP) {
		IOLoop::Instance()->removeHandler(fd);
		return -1;
	}

	if (e.events & EPOLLERR) {
		return -1;
	}

	if (e.events & EPOLLOUT)
	{
		std::cout << "Writing: " << std::endl;
		if(sendto(fd, "test", 5, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			std::cout << "Error writing to socket" << std::endl;
		}

		IOLoop::Instance()->modifyHandler(fd, EPOLLIN);
	}

	if (e.events & EPOLLIN) {
		int numbytes = recvfrom(fd, buffer, 1024, 0 , (struct sockaddr*)&addr, &addrlen);
		printf("\r\033[34m[%s]:\033[34m data lenght = %d\n", addr.sun_path, numbytes);
		if ( numbytes == -1){ 
			printf("recvfrom() error\n");
			exit(1);
		}
		buffer[numbytes] = 0;
		if(numbytes > 0)
			printf("recv:%s\n",buffer);

		//sendto(fd, "test", 5, 0, (struct sockaddr *)&addr, sizeof(addr));
		//IOLoop::Instance()->modifyHandler(fd, EPOLLOUT);
		//IOLoop::Instance()->addHandler(fd, this, EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR);
	}

	return 0;
}

