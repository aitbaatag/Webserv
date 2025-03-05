#include "server_socket.hpp"

void ServerSocket::initialize_socket(int port) {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0)
    throw std::runtime_error("Failed to create socket: " +
                             std::string(strerror(errno)));

  int opt = 1;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    std::cerr << "Failed to set SO_REUSEADDR. Error: " << strerror(errno)
              << std::endl;
    close(socket_fd_);
    return;
  }
  server_address_.sin_family = AF_INET;
  server_address_.sin_port = htons(port);
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

int ServerSocket::get_socket_fd() { return socket_fd_; }

ServerSocket::ServerSocket(int port) : socket_fd_(-1), epoll_fd_(-1) {
  initialize_socket(port);
  bind_socket();
  listen_for_connections();
  set_non_blocking();
  std::cout << "Server is listening on port " << port << std::endl;
}

ServerSocket::~ServerSocket() {
  if (socket_fd_ != -1)
    close(socket_fd_);
}

int ServerSocket::accept_connection() {
  int client_socket = accept(socket_fd_, NULL, NULL);
  if (client_socket < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK)
      throw std::runtime_error("Accept error: " + std::string(strerror(errno)));
    else
      return -1;
  } else {
    if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0) {
      close(client_socket);
      throw std::runtime_error("Failed to set client socket to non-blocking");
    }
  }
  return client_socket;
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

int ServerSocket::getEpollInstanceFd() {
  if (epoll_fd_ == -1) {
    throw std::runtime_error("Epoll instance not created");
  }
  return epoll_fd_;
}
