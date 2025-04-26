#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "../libraries/Libraries.hpp"
#include "../http_client/http_client.hpp"
#include "../Config/Config.hpp"
#include "../Http_Req_Res/Request.hpp"
#include "../Http_Req_Res/Response.hpp"
#include "../cookies/session_manager.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <ctime>
#include <string>

#include <string>
#include <ctime>
#include <sstream>

class Logger
{
	public:
		static std::string get_timestamp()
		{
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			char buffer[20];
			strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
			return std::string(buffer);
		}

	static std::string get_time_only()
	{
		time_t now = time(NULL);
		char buffer[9];
		strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&now));
		return std::string(buffer);
	}

	static std::string info(const std::string &message)
	{
		return "[INFO][" + get_time_only() + "] " + message + "\n";
	}

	static std::string error(const std::string &message)
	{
		return "[ERROR][" + get_time_only() + "] " + message + "\n";
	}

	static std::string trace_http(const std::string &level, const std::string &ip,
			unsigned short port, const std::string &method, const std::string &path, const std::string &http_version,
						int status_code,
						int duration_ms)
	{
		std::ostringstream oss;
		oss << "[" << level << "] " <<
			"[" << get_timestamp() << "] " <<
			"[" << ip << ":" << port << "] " <<
			"\"" << method << " " << path << " " << http_version << "\"" <<
			" → " << status_code << " (" << duration_ms << "ms)";
		return oss.str() + "\n";
	}
};


struct ClientConnectionInfo
{
	int client_socket;
	std::string client_ip;
	uint16_t client_port;
};


class ServerSocket
{
	private:
		int socket_fd_;	// File descriptor for the server socket
		int epoll_fd_;	// File descriptor for epoll instance
		int server_port_;	// Port number the server listens on
		std::string server_host_;
		struct epoll_event events[MAX_EVENTS];	// Array to store epoll events
		struct sockaddr_in server_address_;	// Server address structure
		std::vector<ServerConfig> serverConfig_;	// Server configuration data
		std::map<int, HttpClient*> clients_;	// Map of connected clients

		// delete
		// Response res;

		void initialize_socket();
		void bind_socket();
		void listen_for_connections();
	public:
		ServerSocket();
		~ServerSocket();

		// brief Creates an epoll instance for event monitoring
		void createEpollInstance();

		// Returns the epoll instance file descriptor
		int getEpollInstanceFd() const { return epoll_fd_;}

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
		void handleClientDisconnection(int client_fd);
		void handleClientConnection();
		void handleClientTimeout();

		// Processes epoll events
		void processEpollEvents(struct epoll_event *events, int ready_fd_count);

		// Starts the server
		void startServer();

		// Sets up the server port
		void setupServerPort();
};

#endif