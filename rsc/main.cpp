#include "../Includes/Http_Req_Res/StateMachine.hpp"
#include "../Includes/Http_Req_Res/Request.hpp"
#include "../Includes/http_client/http_client.hpp"
#include "../Includes/Http_Req_Res/Response.hpp"
#include "../Includes/Config/Config.hpp"
#include "../Includes/server/server_socket.hpp"
#include <signal.h>

static bool running = true;


void signal_handler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\nshutting down gracefully..." << std::endl;
        running = false;
    }
}

void addConfigServer(ServerConfig &serversConfig, std::vector<ServerSocket*> &servers)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i]->getServerPort() == serversConfig.port)
		{
			servers[i]->getServerConfig().push_back(serversConfig);
			return ;
      }
   }
	ServerSocket *newServer = new ServerSocket();
	newServer->getServerConfig().push_back(serversConfig);
	newServer->setupServerPort();
	servers.push_back(newServer);
}

/// hierarchical event (optimization)
void startServers(std::vector<ServerSocket*> &servers)
{
	int epfd_master = epoll_create1(0);
	if (epfd_master == -1)
	{
		perror("epoll_create1");
		return;
	}

	for (std::vector<ServerSocket*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		(*it)->startServer();
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.ptr = *it;
		if (epoll_ctl(epfd_master, EPOLL_CTL_ADD, (*it)->getEpollInstanceFd(), &ev) == -1)
		{
			perror("epoll_ctl (add child epoll to master)");
			return;
		}
	}

	struct epoll_event *master_events = new epoll_event[servers.size()];
	while (running)
	{
		int nfds = epoll_wait(epfd_master, master_events, servers.size(), EPOLL_TIMEOUT);
		if (nfds < 0)
		{
			perror("epoll_wait (master)");
			break;
		}
		for (int i = 0; i < nfds; ++i)
		{
			ServerSocket *server = static_cast<ServerSocket*> (master_events[i].data.ptr);
			struct epoll_event events[MAX_EVENTS];
			int ready_fds = epoll_wait(server->getEpollInstanceFd(), events, MAX_EVENTS, 0);
			if (ready_fds > 0)
			{
				for (int j = 0; j < ready_fds; ++j)
				{
					if (events[j].data.fd == server->get_socket_fd())
						server->handleClientConnection();
					else
						server->processEpollEvents(events, ready_fds);
				}
			}
			server->handleClientTimeout();
		}
	}
	delete[] master_events;
	close(epfd_master);
}

void printServerBanner() {
    std::cout << "\033[2J\033[1;1H";
    std::string PEACH_COLOR = "\033[38;2;206;145;120m";
    std::cout << PEACH_COLOR << Color::BOLD << "\n\n" <<
        "     ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗\n" <<
        "     ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║\n" <<
        "     ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║\n" <<
        "     ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝\n" <<
        "     ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ \n" <<
        "      ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  " << 
        Color::RESET << std::endl;
    std::cout << Color::GRAY << "                        HTTP Server Implementation" << Color::RESET << std::endl;
    std::cout << PEACH_COLOR << "                 Made by: alamaoui, kait-baa & orhaddao" << Color::RESET << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
	printServerBanner();
	signal(SIGINT, signal_handler);
	std::vector<ServerSocket*> servers;
	ServerConfigParser configParser;

	try
	{
		configParser.parseConfigFile(argc, argv);
		for (size_t i = 0; i < (configParser.getServers()).size(); i++)
			addConfigServer((configParser.getServers())[i], servers);
		startServers(servers);
		for (size_t i = 0; i < servers.size(); ++i)
			delete servers[i];
		return 0;
	}

	catch (const std::exception &e)
	{
		std::cerr << e.what();
		for (size_t i = 0; i < servers.size(); ++i)
			delete servers[i];
		return 1;
	}
}