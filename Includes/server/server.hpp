#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <cerrno>
#include <arpa/inet.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <string>
#include <map>



class ServerSocket {
	private:
		int socket_fd_;
		int epoll_fd_;
		sockaddr_in server_address_;
		void initialize_socket(int port);
		void bind_socket();
		void listen_for_connections();
		void set_non_blocking();
	public:
		void createEpollInstance();
		int getEpollInstanceFd();
		ServerSocket(int port);
		~ServerSocket();
		int accept_connection();
		int get_socket_fd();
};



#endif