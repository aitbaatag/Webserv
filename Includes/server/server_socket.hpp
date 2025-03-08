#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "../libraries/Libraries.hpp"

#define MAX_EVENTS 10
#define MAX_RECV 2048

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