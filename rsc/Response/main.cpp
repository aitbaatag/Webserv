#include "../../Includes/Http_Req_Res/Response.hpp"

// int main() {
//     Request req("GET", "/index.html", "HTTP/1.1");
    
//     Response res;
    
//     std::string response = res.generateResponse(req);
    
//     std::cout << "Generated HTTP Response:\n" << std::endl;
//     std::cout << response << std::endl;
    
//     return 0;
// }

#include "../../Includes/Http_Req_Res/Request.hpp"
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
                        std::map<int, HttpClient> &clients, HttpRequest &req, Response &res) {

  for (int i = 0; i < ready_fd_count; ++i) {
    {
    int client_fd = events[i].data.fd;

      if (events[i].events & EPOLLIN) {
        // Handle req side
        //        std::cout << clients[events[i].data.fd].request_buffer_ <<
        //        std::endl;
        req.parseIncrementally(clients[events[i].data.fd]);
        clients[events[i].data.fd].append_to_request();
      }
      if ((events[i].events & EPOLLOUT) && clients[events[i].data.fd].get_request_status() == Complete) {
        std::string response = res.generateResponse(clients[events[i].data.fd]);
        // Send response to the client
      ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
      if (bytes_sent < 0) {
        std::cerr << "Error sending response to client FD " << client_fd << std::endl;
      } else {
        std::cout << "Sent response to client FD " << client_fd << std::endl;
      }

      // Close connection after response is sent
      close(client_fd);
      clients.erase(client_fd);
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

  return 0;
}
