#include "http_client.hpp"


HttpClient::HttpClient(int socket_fd)
{
	socket_fd_ = socket_fd;
	request_status_ = InProgress;
	response_status_ = InProgress;
}

int HttpClient::get_socket_fd()
{
	return socket_fd_;
}

Status HttpClient::get_request_status()
{
	return request_status_;
}

Status HttpClient::get_response_status()
{
	return response_status_;
}

void HttpClient::append_to_request()
{
	char buffer[MAX_RECV] = {};
	
	size_t bytes_received = recv(socket_fd_, buffer, MAX_RECV - 1, 0);
	if (bytes_received < 0)
		throw std::runtime_error(std::string("recv failed: ") + strerror(errno));
	else if (bytes_received == 0)
		return ;
	buffer[bytes_received] = '\0';
	request_buffer_ += buffer;
	std::cout << "read from client: " << socket_fd_ << " => " << request_buffer_ << std::endl;
}

void HttpClient::registerEpollEvents(int epoll_fd_)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = socket_fd_;

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &ev) == -1) {
		throw std::runtime_error("Failed to add FD to epoll: " + std::string(strerror(errno)));
	}
}

