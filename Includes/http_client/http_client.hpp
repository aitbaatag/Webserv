
#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../Http_Req_Res/StateMachine.hpp"
#include "../libraries/Libraries.hpp"
#include "../Http_Req_Res/Response.hpp"
#include"../Http_Req_Res/Request_Struct.hpp"


// Content-Type: text/html; charset=UTF-8; version=1.0
enum Status { InProgress, Complete, Failed, Disc };

class HttpClient {
public:
  int socket_fd_;
  std::string client_ip;
  uint16_t client_port;
  char buffer[MAX_RECV];
  size_t bytes_received;
  std::string response_buffer_;
  Status request_status_;
  Status response_status_;
  int pos_;
  size_t time_client_;
  unsigned long long	time_start_;


public:
  StateMachine SMrequest;
  Request Srequest;
	Response res;

  HttpClient(int client_socket, std::string client_ip, uint16_t client_port);
  HttpClient() {};
  int get_socket_fd();
  Status get_request_status();
  Status get_response_status();
  void append_to_request();
  void registerEpollEvents(int epoll_fd_);
  void update_pos(int new_pos);
  int get_pos() const;
  char *get_request_buffer();
  void set_request_status(Status status);
  void set_response_status(Status status);
  size_t get_client_time() { return time_client_; };
  std::string get_client_ip() { return client_ip; };
  
};

#endif
