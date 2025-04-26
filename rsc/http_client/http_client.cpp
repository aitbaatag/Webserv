
#include "../../Includes/http_client/http_client.hpp"

size_t get_current_time() { return (size_t)time(NULL); }

HttpClient::HttpClient(int client_socket, std::string client_ip, uint16_t client_port) : SMrequest(){
  socket_fd_ = client_socket;
  pos_ = 0;
  request_status_ = InProgress;
  response_status_ = InProgress;
  this->client_ip = client_ip;
  this->client_port = client_port;
  time_start_ = time(NULL);
  time_client_ = get_current_time();
}

int HttpClient::get_socket_fd() { return socket_fd_; }

Status HttpClient::get_request_status() { return request_status_; }

Status HttpClient::get_response_status() { return response_status_; }

void HttpClient::append_to_request() {
  pos_ = 0;
  bytes_received = recv(socket_fd_, buffer, MAX_RECV - 1, 0);
  if (bytes_received <= 0) {
    request_status_ = Disc;
    return;
  }
  time_client_ = time(NULL);
}

void HttpClient::registerEpollEvents(int epoll_fd_) {
  epoll_event ev;
  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.fd = socket_fd_;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket_fd_, &ev) == -1) {
    throw std::runtime_error("Failed to add FD to epoll: " +
                             std::string(strerror(errno)));
  }
}

void HttpClient::update_pos(int new_pos) { pos_ = new_pos; }

int HttpClient::get_pos() const { return pos_; }

char *HttpClient::get_request_buffer() { return buffer; }

void HttpClient::set_request_status(Status status) { request_status_ = status; }

void HttpClient::set_response_status(Status status) {
  response_status_ = status;
}
