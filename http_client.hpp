#ifndef HTTP_CLIENT_HPP
# define HTTP_CLIENT_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <string>
#include <cerrno>

#define MAX_EVENTS 10
#define MAX_RECV 2



enum Status {
	InProgress,
	Complete,
	Failed
};

class HttpClient
{
	private:

		int socket_fd_;
		std::string request_buffer_;
		std::string response_buffer_;
		Status request_status_;
		Status response_status_;
	public:
		HttpClient(int socket_fd);
		HttpClient() {};
		int get_socket_fd();
		Status get_request_status();
		Status get_response_status();
		void append_to_request();
		void registerEpollEvents(int epoll_fd_);
};


#endif