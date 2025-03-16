#include "../../Includes/server/server_socket.hpp"

void ServerSocket::initialize_socket() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0)
    throw std::runtime_error("Failed to create socket: " +
                             std::string(strerror(errno)));

  int opt = 1;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    std::cerr << "Failed to set socket options: " << strerror(errno)
              << std::endl;
    close(socket_fd_);
    return;
  }
  server_address_.sin_family = AF_INET;
  server_address_.sin_port = htons(server_port_);
  server_address_.sin_addr.s_addr = htonl(INADDR_ANY);
}

void ServerSocket::bind_socket() {
  if (bind(socket_fd_, (struct sockaddr *)&server_address_,
           sizeof(server_address_)) < 0)
    throw std::runtime_error("Failed to bind socket: " +
                             std::string(strerror(errno)));
}

void ServerSocket::listen_for_connections() {
  if (listen(socket_fd_, SOMAXCONN) < 0)
    throw std::runtime_error("Failed to listen on socket: " +
                             std::string(strerror(errno)));
}

void ServerSocket::set_non_blocking() {
  if (fcntl(socket_fd_, F_SETFL, O_NONBLOCK) < 0)
    throw std::runtime_error("Failed to set socket to non-blocking: " +
                             std::string(strerror(errno)));
}

ServerSocket::~ServerSocket() {
  if (socket_fd_ != -1)
    close(socket_fd_);
}

ClientConnectionInfo ServerSocket::accept_connection() {
  sockaddr_in client_addr;
  ClientConnectionInfo client;
  socklen_t client_len;
  client_len = sizeof(client_addr);

  client.client_socket =
      accept(socket_fd_, (sockaddr *)&client_addr, &client_len);
  if (client.client_socket < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK)
      throw std::runtime_error("Accept error: " + std::string(strerror(errno)));
    else {
      client.client_socket = -1;
      return client;
    }
  } else {
    if (fcntl(client.client_socket, F_SETFL, O_NONBLOCK) < 0) {
      close(client.client_socket);
      throw std::runtime_error("Failed to set client socket to non-blocking");
    }
  }
  inet_ntop(AF_INET, &(client_addr.sin_addr), client.client_ip,
            INET_ADDRSTRLEN);
  return client;
}

void ServerSocket::createEpollInstance() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
  epoll_fd_ = epoll_create(10);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("Failed to create epoll instance: " +
                             std::string(strerror(errno)));
  }
}

void ServerSocket::handleClientDisconnection(int client_fd) {
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, NULL) == -1)
    perror("Failed to remove client from epoll");
  std::map<int, HttpClient>::iterator it = clients_.find(client_fd);
  if (it != clients_.end())
    clients_.erase(it);
  close(client_fd);
}

ServerSocket::ServerSocket() {
  socket_fd_ = -1;
  epoll_fd_ = -1;
  server_port_ = -1;
}

void ServerSocket::startServer() {
  initialize_socket();
  bind_socket();
  listen_for_connections();
  set_non_blocking();
  createEpollInstance();
}

void ServerSocket::setupServerPort() {
  if (serverConfig_.size() != 0)
    server_port_ = serverConfig_[0].port;
}

void ServerSocket::processEpollEvents(int ready_fd_count) {
  for (int i = 0; i < ready_fd_count; i++) {
    {
      if ((events[i].events & EPOLLIN) &&
          clients_[events[i].data.fd].get_request_status() == InProgress) {
        clients_[events[i].data.fd].append_to_request();
        req.parseIncrementally(clients_[events[i].data.fd]);
      }

      if ((events[i].events & EPOLLOUT) &&
          clients_[events[i].data.fd].get_request_status() == Complete) {
        res.generateResponse(clients_[events[i].data.fd], events[i].data.fd);
        if (clients_[events[i].data.fd].get_response_status() == Complete) {
          clients_.erase(events[i].data.fd);
          close(events[i].data.fd);
        }
      }
    }
  }
}

void ServerSocket::handleClientConnection() {
  ClientConnectionInfo client = accept_connection();
  if (client.client_socket > 0) {
    clients_[client.client_socket] = HttpClient(client.client_socket);
    clients_[client.client_socket].client_ip = client.client_ip;
    clients_[client.client_socket].registerEpollEvents(epoll_fd_);
  }
  int ready_fd_count = epoll_wait(epoll_fd_, events, MAX_EVENTS, 0);
  if (ready_fd_count < 0)
    throw std::runtime_error("epoll_wait failed: " +
                             std::string(strerror(errno)));
  processEpollEvents(ready_fd_count);
}
