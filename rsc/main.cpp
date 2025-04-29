#include "../Includes/Http_Req_Res/StateMachine.hpp"
#include "../Includes/Http_Req_Res/Request.hpp"
#include "../Includes/http_client/http_client.hpp"
#include "../Includes/Http_Req_Res/Response.hpp"
#include "../Includes/Config/Config.hpp"
#include "../Includes/server/server_socket.hpp"
#include <signal.h>
#include "../Includes/utlis/utils.hpp"

static bool running = true;


void signal_handler(int signum)
{
    if (signum == SIGINT) {
        std::cout <<  Color::BLUE + Color::BOLD + "\n[INFO] " + Color::RESET + 
		Color::CYAN + " [" + Logger::get_timestamp() + "]" + Color::RESET +   " Shutting down gracefully..." << Color::RESET << std::endl;
        running = false;
    }
}



void startServers(std::vector<ServerSocket*> &servers, int epfdMaster)
{
	epoll_event events[MAX_EVENTS];
	for (std::vector<ServerSocket*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		(*it)->startServer();
	}
	
	std::map<int, EpollEventContext*> &FileDescriptorList = servers[0]->getServerConfigParser()->getFileDescriptorList();

	while (running)
	{
		int ready_fds = epoll_wait(epfdMaster, events, MAX_EVENTS, 100);
		if (ready_fds > 0)
		{
			for (int j = 0; j < ready_fds; ++j)
			{
				EpollEventContext *context = (EpollEventContext*) events[j].data.ptr;
				if (context->descriptorType == ServerSocketFd)
					context->serverSocket->handleClientConnection();
				else if (context->descriptorType == ClientSocketFd)
					processEpollEvents(&events[j], context->httpClient, epfdMaster);
			}
		}
		handleClientTimeouts(FileDescriptorList, epfdMaster);
	}
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
			addConfigServer((configParser.getServers())[i], servers, configParser.getEpfdMaster());
		startServers(servers, configParser.getEpfdMaster());
		for (size_t i = 0; i < servers.size(); ++i)
		{
            delete servers[i];
        }
		return 0;
	}

	catch (const std::exception &e)
	{
		std::cerr << e.what();
		for (size_t i = 0; i < servers.size(); ++i) {
            delete servers[i];
        }	
		return 1;
	}
}