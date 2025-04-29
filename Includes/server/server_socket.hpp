#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "../libraries/Libraries.hpp"
#include "../http_client/http_client.hpp"
#include "../Config/Config.hpp"
#include "../Http_Req_Res/Request.hpp"
#include "../Http_Req_Res/Response.hpp"
#include "../cookies/session_manager.hpp"



struct ClientConnectionInfo
{
	int client_socket;
	std::string client_ip;
	uint16_t client_port;
};


class ServerSocket
{
	private:
		int							socket_fd_;	// File descriptor for the server socket
		int							server_port_;	// Port number the server listens on
		std::string					server_host_;
		std::vector<ServerConfig>	serverConfig_;	// Server configuration data

		//////////
		int							epfdMaster;
		ServerConfigParser			*scp;

		void initialize_socket();
		void bind_socket();
		void listen_for_connections();
	public:
		ServerSocket();
		ServerSocket(int epfdMaster) {this->epfdMaster = epfdMaster;};
		~ServerSocket();

		// brief Creates an epoll instance for event monitoring

		// Returns the epoll instance file descriptor
		// int getEpollInstanceFd() const { return epoll_fd_;}

		// Accepts a new client connection
		ClientConnectionInfo accept_connection();


		// Returns the server socket file descriptor
		int get_socket_fd() const {return socket_fd_;}

		// Returns the server port number
		int getServerPort() const { return server_port_; }

		// Returns reference to server configuration vector
		std::vector<ServerConfig> &getServerConfig() {
		return serverConfig_;
		}

		// Handles client disconnection && Connection && timeout 
		void handleClientConnection();

		// Processes epoll events

		// Starts the server
		void startServer();

		// Sets up the server port
		void setupServerPort();

		void setServerConfigParser(ServerConfigParser *p) {this->scp = p;};
		ServerConfigParser	*getServerConfigParser() {return scp;}
};

#endif