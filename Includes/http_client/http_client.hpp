#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP
#include "../Http_Req_Res/StateMachine.hpp"
#include "../libraries/Libraries.hpp"
#include "../Http_Req_Res/Response.hpp"
#include"../Http_Req_Res/Request_Struct.hpp"

class ServerSocket;
enum Status
{
	InProgress, Complete, Failed, Disc
};

class HttpClient
{
	public:
		// Connection data
		int                 socket_fd_;
		std::string         client_ip;
		uint16_t            client_port;

		// Request/Response state
		char                buffer[MAX_RECV];
		size_t              bytes_received;
		std::string         response_buffer_;
		Status              request_status_;
		Status              response_status_;
		int                 pos_;

		// Timing information
		size_t              time_client_;
		unsigned long long  time_start_;

		// track read open
		std::set<int>		readTrack;
		std::set<int>		writeTrack;


	public:
		// Request/Response handling objects
		StateMachine        SMrequest;
		Request             Srequest;
		Response            res;
		ServerSocket		*server;
		Route               *route;
		ServerConfig        *server_config;


	  	// Getters
	  	int                 get_socket_fd();
		int                 get_pos() const;
		Status              get_request_status();
		Status              get_response_status();
		char                *get_request_buffer();
		size_t              get_client_time();
		std::string         get_client_ip();

		// Setters
		void                set_request_status(Status status);
		void                set_response_status(Status status);
		void                update_pos(int new_pos);

		// Operations
		void                append_to_request();
		void				reset();

		std::set<int>&		getReadTrack() {return readTrack;};
		std::set<int>&		getWriteTrack() {return writeTrack;};

		// Constructors
	  	HttpClient(int client_socket, std::string client_ip, uint16_t client_port);
	  	HttpClient();
		~HttpClient();

		void deleteFileEpoll(int fd);
		void registerFileEpoll(int fd);
		
};

void processEpollEvents(epoll_event *event, HttpClient *c, int epfdMaster);
void handleClientDisconnection(HttpClient *c, int epfdMaster);
void cleanAllClientServer(std::map<int, EpollEventContext*> &FileDescriptorList);
void handleClientTimeouts(std::map<int, EpollEventContext*> &FileDescriptorList, int epfdMaster);
void addConfigServer(ServerConfig &serversConfig, std::vector<ServerSocket*> &servers, int epfdMaster);

#endif