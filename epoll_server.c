
#include <sys/epoll.h>

#include <unistd.h>

#include <fcntl.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <errno.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>


#define MAX_EVENTS 1024
#define BUF_SIZE 1024


int set_nonblocking(int fd){
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1){
		return -1;
	}
	
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1){
		return -1;
	}
	return 0;
}


int main(int argc, char *argv[])
{
	if (argc <= 2){
		printf("Usage: ip port\r\n");
		return 1;
	}
	
	const char *ip = argv[1];
	int port = atoi(argv[2]);
	
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1){
		perror("socket");
		return 1;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	
	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		perror("bind");
		return 1;
	}
	
	if (listen(listen_fd, SOMAXCONN) == -1){
		perror("listen");
		return 1;
	}
	
	int epoll_fd = epoll_create(1);	//参数必须大于0，值在内部会被忽略
	if (epoll_fd == -1){
		perror("epoll_create");
		return 1;
	}
	
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = listen_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1){
		perror("epoll_ctl");
		return 1;
	}
	
	struct epoll_event events[MAX_EVENTS];
	char buf[BUF_SIZE];
	while(1){
		// int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
		//
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n == -1){
			perror("epoll_wait");
			break;
		}
		
		for (int i = 0; i < n; ++i){
			int fd = events[i].data.fd;
			if (fd == listen_fd){
				printf("=================client come in\r\n");
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				
				int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
				if (conn_fd == -1){
					perror("accept");
					continue;
				}
				
				set_nonblocking(conn_fd);
				event.events = EPOLLIN | EPOLLET;
				event.data.fd = conn_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) == -1){
					perror("epoll_ctl");
					close(conn_fd);
					continue;
				}
			}
			else{
				while(1){
					int nread = read(fd, buf, BUF_SIZE);
					if (nread == -1 && errno == EAGAIN){
						printf("=================have some data to be read\r\n");
						break;
					}
					if (nread == 0 || (nread == -1 && errno != EINTR && errno != EAGAIN)){
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
						close(fd);
						printf("=================nothing to be read\r\n");
						break;
					}
					write(STDOUT_FILENO, buf, nread);
				}
			}
		}
	}
	
	return 0;
}
