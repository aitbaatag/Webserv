#include "../../Includes/server/server_socket.hpp"



void ServerSocket::initialize_socket()
{
	socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd_ < 0)
		throw std::runtime_error(Logger::error("Failed to create socket: " + std::string(strerror(errno))));

	int opt = 1;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(socket_fd_);
		throw std::runtime_error(Logger::error("Failed to set socket options: " + std::string(strerror(errno))));
		return ;
	}
}

void ServerSocket::bind_socket()
{
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(server_host_.c_str(), NULL, &hints, &res);
	if (status != 0)
	{
		throw std::runtime_error(Logger::error("Failed to resolve host '" + server_host_ + "': " +
			std::string(gai_strerror(status))));
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port_);
	server_address.sin_addr.s_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr.s_addr;

	struct in_addr addr;
	addr.s_addr = server_address.sin_addr.s_addr;
	std::string resolved_ip = inet_ntoa(addr);
	freeaddrinfo(res);

	if (bind(socket_fd_, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
	{
		throw std::runtime_error(Logger::error("Bind failed: " + std::string(strerror(errno))));
	}
}

void ServerSocket::listen_for_connections()
{
	if (listen(socket_fd_, SOMAXCONN) < 0)
		throw std::runtime_error(Logger::error("Failed to listen on socket: " + std::string(strerror(errno))));
}

void ServerSocket::set_non_blocking()
{
	if (fcntl(socket_fd_, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error(Logger::error("Failed to set socket to non-blocking: " + std::string(strerror(errno))));
}

ServerSocket::~ServerSocket()
{
	if (socket_fd_ != -1)
		close(socket_fd_);
}

ClientConnectionInfo ServerSocket::accept_connection()
{
	sockaddr_in client_addr;
	ClientConnectionInfo client;
	socklen_t client_len;
	client_len = sizeof(client_addr);

	client.client_socket = accept(socket_fd_, (sockaddr*) &client_addr, &client_len);
	if (client.client_socket < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw std::runtime_error(Logger::error("Accept error: " + std::string(strerror(errno))));
		else
		{
			client.client_socket = -1;
			return client;
		}
	}
	else
	{
		if (fcntl(client.client_socket, F_SETFL, O_NONBLOCK) < 0)
		{
			close(client.client_socket);
			throw std::runtime_error(Logger::error("Failed to set client socket to non-blocking: " + std::string(strerror(errno))));
		}
	}

	inet_ntop(AF_INET, &(client_addr.sin_addr), client.client_ip, INET_ADDRSTRLEN);
	return client;
}

void ServerSocket::createEpollInstance()
{
	if (epoll_fd_ != -1)
	{
		close(epoll_fd_);
	}

	epoll_fd_ = epoll_create(10);
	if (epoll_fd_ == -1)
	{
		throw std::runtime_error(Logger::error("Failed to create epoll instance: " + std::string(strerror(errno))));
	}
}


ServerSocket::ServerSocket()
{
	socket_fd_ = -1;
	epoll_fd_ = -1;
	server_port_ = -1;
}

void ServerSocket::startServer()
{
	try
	{
		initialize_socket();
		bind_socket();
		listen_for_connections();
		set_non_blocking();
		createEpollInstance();

		std::string url = "http://" + (server_host_ == "0.0.0.0" ? "localhost" : server_host_) +
			":" + std::to_string(server_port_) + "/";
		std::string message = Logger::info("Serving HTTP on " + server_host_ + " port " + std::to_string(server_port_) + " (" + url + ")");
		std::cout << message;
	}

	catch (const std::exception &e)
	{
		if (socket_fd_ != -1)
		{
			close(socket_fd_);
			socket_fd_ = -1;
		}

		if (epoll_fd_ != -1)
		{
			close(epoll_fd_);
			epoll_fd_ = -1;
		}

		throw;
	}
}

void ServerSocket::setupServerPort()
{
	if (serverConfig_.size() != 0)
	{
		server_port_ = serverConfig_[0].port;
		server_host_ = serverConfig_[0].host;
	}
}

void ServerSocket::processEpollEvents(int ready_fd_count)
{
	for (int i = 0; i < ready_fd_count; i++)
	{
		{
			if ((events[i].events &EPOLLIN) &&
				clients_[events[i].data.fd].get_request_status() == InProgress)
			{
				clients_[events[i].data.fd].append_to_request();
				req.parseIncrementally(clients_[events[i].data.fd]);
			}

			if ((events[i].events &EPOLLOUT) &&
				clients_[events[i].data.fd].get_request_status() == Complete)
			{
				res.response_handler(clients_[events[i].data.fd], events[i].data.fd, serverConfig_);
				if (clients_[events[i].data.fd].get_response_status() == Complete) {
					handleClientDisconnection(events[i].data.fd);
					std::cout << Logger::info("Client " + std::to_string(events[i].data.fd) + " disconnected");
				}
			}
		}
	}
	
}

void ServerSocket::handleClientConnection()
{
	ClientConnectionInfo client = accept_connection();
	if (client.client_socket > 0)
	{
		clients_[client.client_socket] = HttpClient(client.client_socket);
		clients_[client.client_socket].client_ip = client.client_ip;
		std::cout << Logger::info("Client " +  std::to_string(client.client_socket) + " connected from " + client.client_ip + " on port " + std::to_string(server_port_));
		clients_[client.client_socket].registerEpollEvents(epoll_fd_);
	}

	int ready_fd_count = epoll_wait(epoll_fd_, events, MAX_EVENTS, 0);
	if (ready_fd_count < 0)
		throw std::runtime_error(Logger::error("epoll_wait failed: " + std::string(strerror(errno))));
	processEpollEvents(ready_fd_count);
}

void ServerSocket::handleClientDisconnection(int client_fd)
{
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, NULL) == -1)
		Logger::error("Failed to remove client " + std::to_string(client_fd) + " from epoll: " + std::string(strerror(errno)));

	std::map<int, HttpClient>::iterator it = clients_.find(client_fd);
	if (it != clients_.end())
	{
		try {
			clients_.erase(it);
		}
		catch (const std::exception &e) {
			Logger::error("Failed to remove client " + std::to_string(client_fd) + " from clients map: " + std::string(e.what()));
		}
	}
	else
		Logger::error("Client " + std::to_string(client_fd) + " not found in clients map");

	if (close(client_fd) == -1)
		Logger::error("Failed to close client socket " + std::to_string(client_fd) + ": " + std::string(strerror(errno)));
}


void ServerSocket::handleClientTimeout()
{
	std::map<int, HttpClient>::iterator it = clients_.begin();

	while (it != clients_.end())
	{
		size_t current_time = time(NULL);
		size_t client_time = it->second.get_client_time();

		if ((current_time - client_time) > TIMEOUT)
		{
			int client_fd = it->first;
			std::string client_ip = it->second.get_client_ip();
			++it;
			std::cout << Logger::info("Client " + std::to_string(client_fd) + " with IP " + client_ip + " timed out after " + std::to_string(TIMEOUT) + " seconds");
			handleClientDisconnection(client_fd);
		}
		else
		{ ++it;
		}
	}
}
