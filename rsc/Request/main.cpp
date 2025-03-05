#include "../../Includes/Http_Req_Res/Request.hpp"
<<<<<<< HEAD
#include "../../Includes/Http_Req_Res/Response.hpp"
#include "../../Includes/http_client/http_client.hpp"
#include "../../rsc/Request/server_socket.hpp"
#include <sys/epoll.h>

void handleNewConnection(int client_fd, int epfd,
                         std::map<int, HttpClient> &clients) {
  try {
    clients[client_fd] = HttpClient(client_fd);
    clients[client_fd].registerEpollEvents(epfd);
    std::cout << "Accepted new connection: client FD " << client_fd
              << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    clients.erase(client_fd);
    if (client_fd >= 0)
      close(client_fd);
  }
}

void processEpollEvents(int ready_fd_count, struct epoll_event *events,
                        std::map<int, HttpClient> &clients, HttpRequest &req,
                        Response &res) {

  for (int i = 0; i < ready_fd_count; ++i) {
    {
      if ((events[i].events & EPOLLIN) &&
          clients[events[i].data.fd].get_request_status() == InProgress) {
        clients[events[i].data.fd].append_to_request();
        req.parseIncrementally(clients[events[i].data.fd]);
      }

      if ((events[i].events & EPOLLOUT) &&
          clients[events[i].data.fd].get_request_status() == Complete) {
        res.generateResponse(clients[events[i].data.fd], events[i].data.fd);
        if (clients[events[i].data.fd].get_response_status() == Complete) {
          clients.erase(events[i].data.fd);
          close(events[i].data.fd);
          std::cout << "Closed connection: client FD " << events[i].data.fd
                    << std::endl;
        }
      }
    }
  }
}

int main() {
  try {
    epoll_event events[MAX_EVENTS] = {};
    ServerSocket server(8081);
    HttpRequest req;
    Response res;
    server.createEpollInstance();
    int epfd = server.getEpollInstanceFd();
    std::map<int, HttpClient> clients;

    while (true) {
      int client_fd = server.accept_connection();
      if (client_fd >= 0)
        handleNewConnection(client_fd, epfd, clients);
      int ready_fd_count = epoll_wait(epfd, events, MAX_EVENTS, 0);
      if (ready_fd_count < 0)
        throw std::runtime_error("epoll_wait failed: " +
                                 std::string(strerror(errno)));
      processEpollEvents(ready_fd_count, events, clients, req, res);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

=======
#include <iostream>
#include <map>
#include <sstream>
#include <string>

void printRequest(const HTTPRequest &req) {
  std::cout << "\n=== Request Details ===" << std::endl;
  std::cout << "Method: " << req.method << std::endl;
  std::cout << "URI: " << req.uri << std::endl;
  std::cout << "Version: " << req.httpVersion << std::endl;

  std::cout << "\n--- Headers ---" << std::endl;
  std::map<std::string, std::string>::const_iterator it;
  for (it = req.headers.begin(); it != req.headers.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }

  if (!req.body.empty()) {
    std::cout << "\n--- Body ---" << std::endl;
    std::cout << req.body << std::endl;
  }
  std::cout << "==================\n" << std::endl;
}

int main() {

  // Test 1: Simple GET request
  std::cout << "Test 1: Simple GET request" << std::endl;
  std::string getRequest = "GET /index.html HTTP/1.1\r\n"
                           "Host: localhost:8080\r\n"
                           "User-Agent: Mozilla/5.0\r\n"
                           "Accept: text/html\r\n"
                           "\r\n";
  HttpRequest parser(getRequest);
  HTTPRequest req1 = parser.parseRequest(getRequest);
  printRequest(req1);
  // Test 2: POST request with body
  std::cout << "Test 2: POST request with body" << std::endl;
  std::string postRequest =
      "POST /submit HTTP/1.1\r\n"
      "Host: localhost:8080\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: 25\r\n"
      "\r\n"
      "username=john&password=123";

  HTTPRequest req2 = parser.parseRequest(postRequest);
  printRequest(req2);

  // Test 3: DELETE request
  std::cout << "Test 3: DELETE request" << std::endl;
  std::string deleteRequest = "DELETE /file.txt HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "\r\n";

  HTTPRequest req3 = parser.parseRequest(deleteRequest);
  printRequest(req3);

  // Test 4: Malformed request (missing HTTP version)
  std::cout << "Test 4: Malformed request" << std::endl;
  try {
    std::string malformedRequest = "GET /index.html\r\n"
                                   "Host: localhost:8080\r\n"
                                   "\r\n";

    HTTPRequest req4 = parser.parseRequest(malformedRequest);
    printRequest(req4);
  } catch (const std::exception &e) {
    std::cout << "Error caught (as expected): " << e.what() << std::endl;
  }

  // Test 5: Request with multiple headers
  std::cout << "Test 5: Request with multiple headers" << std::endl;
  std::string complexRequest = "GET /api/data HTTP/1.1\r\n"
                               "Host: localhost:8080\r\n"
                               "User-Agent: Mozilla/5.0\r\n"
                               "Accept: text/html,application/xhtml+xml\r\n"
                               "Accept-Language: en-US,en;q=0.9\r\n"
                               "Accept-Encoding: gzip, deflate\r\n"
                               "Connection: keep-alive\r\n"
                               "Cookie: sessionId=abc123\r\n"
                               "\r\n";

  HTTPRequest req5 = parser.parseRequest(complexRequest);
  printRequest(req5);

  // Test 6: POST request with chunked encoding
  std::cout << "Test 6: POST request with chunked encoding" << std::endl;
  std::string chunkedRequest = "POST /upload HTTP/1.1\r\n"
                               "Host: localhost:8080\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "7\r\n"
                               "Mozilla\r\n"
                               "9\r\n"
                               "Developer\r\n"
                               "0\r\n"
                               "\r\n";

  HTTPRequest req6 = parser.parseRequest(chunkedRequest);
  printRequest(req6);

  // Test 7: Request with empty body but Content-Length: 0
  std::cout << "Test 7: Request with Content-Length: 0" << std::endl;
  std::string emptyBodyRequest = "POST /empty HTTP/1.1\r\n"
                                 "Host: localhost:8080\r\n"
                                 "Content-Length: 0\r\n"
                                 "\r\n";

  HTTPRequest req7 = parser.parseRequest(emptyBodyRequest);
  printRequest(req7);

>>>>>>> main
  return 0;
}
