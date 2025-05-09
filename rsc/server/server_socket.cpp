#include "../../Includes/server/server_socket.hpp"
#include "../../Includes/utlis/utils.hpp"


void ServerSocket::initialize_socket()
{
	socket_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socket_fd_ < 0)
		throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));

	int opt = 1;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(socket_fd_);
		throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
		return;
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
		throw std::runtime_error("Failed to resolve host '" + server_host_ + "': " + std::string(gai_strerror(status)));
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
		throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
	}
}

void ServerSocket::listen_for_connections()
{
	if (listen(socket_fd_, SOMAXCONN) < 0)
		throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
}

ServerSocket::~ServerSocket()
{
	close_fd(socket_fd_);
}

ClientConnectionInfo ServerSocket::accept_connection()
{
    sockaddr_in client_addr;
    ClientConnectionInfo client;
    socklen_t client_len = sizeof(client_addr);

    client.client_socket = accept(socket_fd_, (sockaddr*) &client_addr, &client_len);
    if (client.client_socket < 0)
    {
        throw std::runtime_error("accept() failed: " + std::string(strerror(errno)));
    }

    char ip_buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_addr.sin_addr), ip_buffer, INET_ADDRSTRLEN))
        client.client_ip = ip_buffer;
    else
        client.client_ip = "Unknown";

    client.client_port = ntohs(client_addr.sin_port);

    return client;
}




ServerSocket::ServerSocket()
{
	socket_fd_ = -1;
	server_port_ = -1;
}

void ServerSocket::startServer()
{
	try
	{
		initialize_socket();
		bind_socket();
		listen_for_connections();

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd_;
		ev.data.ptr = EpollEventContext::createServerData(socket_fd_, this);
		scp->getFileDescriptorList()[socket_fd_] = (EpollEventContext *) ev.data.ptr;
		if (epoll_ctl(epfdMaster, EPOLL_CTL_ADD, socket_fd_, &ev) < 0)
		{
			throw std::runtime_error("Failed to add server socket to epoll");
		}

		std::string url = "http://" + (server_host_ == "0.0.0.0" ? "localhost" : server_host_) +
			":" + to_string(server_port_) + "/";
		std::string message = Logger::info("Serving HTTP on " + server_host_ + " port " + to_string(server_port_) + " (" + url + ")");
		std::cout << message;
	}

	catch (const std::exception &e)
	{
		if (socket_fd_ != -1)
		{
			close_fd(socket_fd_);
		}
		Logger::error("[startServer] " + std::string(e.what()));
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


void ServerSocket::handleClientConnection()
{
	ClientConnectionInfo client;
	client.client_socket = -1;
	try
	{
		HttpClient * hc;
		client = accept_connection();
		if (client.client_socket > 0)
		{
			hc = new HttpClient(client.client_socket, client.client_ip, client.client_port);
			hc->client_ip = client.client_ip;
			hc->server = this;

			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLOUT;
			ev.data.fd = client.client_socket;
			ev.data.ptr = EpollEventContext::createClientData(client.client_socket, hc);
			scp->getFileDescriptorList()[client.client_socket] = (EpollEventContext*) ev.data.ptr;
			if (epoll_ctl(epfdMaster, EPOLL_CTL_ADD, client.client_socket, &ev) < 0)
			{
				throw std::runtime_error("Failed to add client to epoll");
			}
		}
	}

	catch (const std::exception &e)
	{
		if (client.client_socket > 0)
		{
			close_fd(client.client_socket);
		}
		Logger::error("[handleClientConnection] " + std::string(e.what()));
	}
}




