#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/http_client/http_client.hpp"
#include "server_socket.hpp"
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
                        std::map<int, HttpClient> &clients, HttpRequest &req) {

  for (int i = 0; i < ready_fd_count; ++i) {
    {
      if (events[i].events & EPOLLIN) {
        // Handle req side
        // req.parseIncrementally(clients[events[i].data.fd]);
        clients[events[i].data.fd].append_to_request();
      }
      if ((events[i].events & EPOLLOUT) &&
          clients[events[i].data.fd].get_request_status() == Complete) {
        // req.printRequestLine(clients[events[i].data.fd]);
        // Handle writable events
      }
    }
  }
}

int main() {
  try {
    epoll_event events[MAX_EVENTS] = {};
    ServerSocket server(8081);
    HttpRequest req;
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
      processEpollEvents(ready_fd_count, events, clients, req);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
