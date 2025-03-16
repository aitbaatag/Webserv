
#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../Http_Req_Res/StateMachine.hpp"
#include "../libraries/Libraries.hpp"
#include <cstddef>

// Content-Type: text/html; charset=UTF-8; version=1.0
enum Status { InProgress, Complete, Failed, Disc };
struct Request {
  std::string method = "";
  std::string uri = "";
  std::string path = "";
  std::string version = "";
  std::string query;    // the query string of the request like ?name=ahmed
  std::string fragment; // the fragment of the request like #section1
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> formData;
  std::string field_name;
  std::string field_body;
  std::string chunkSizeStr;
  int chunkSize;
  std::string chunkData;
  std::string media_type;
  size_t chunk_size;
  size_t body_length;
  std::string boundary;
  std::string charset;
  std::string Content_Type;
  int error_status = 0; // the error status of the request
  int file_fd;
  std::string filename;
  std::string currentHeader;
  std::string currentData;
  int body_read = 0;
  std::fstream fileStream;
  std::fstream tmpFileStream;
};
class HttpClient {
public:
  int socket_fd_;
  std::string client_ip;
  std::string request_buffer_;
  char buffer[MAX_RECV] = {};
  size_t bytes_received;
  std::string response_buffer_;
  Status request_status_;
  Status response_status_;
  int pos_; // position in the request_buffer_ to avoid re-parsing the same data

public:
  size_t poss = 0;
  StateMachine SMrequest;
  Request Srequest;
  HttpClient(int socket_fd);
  HttpClient(){};
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
};

#endif
