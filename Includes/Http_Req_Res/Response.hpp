#ifndef RESPONSE_HPP
# define RESPONSE_HPP
#include "../libraries/Libraries.hpp"
#include "../Config/Config.hpp"
#include "../Http_Req_Res/Request_Struct.hpp"


enum SendState {
	SEND_IDLE,
	SEND_READING,
	SEND_SENDING,
	SEND_DONE
};


enum HandlerState {
    HSTATE_ERROR_CHECK,
    HSTATE_REDIRECT_CHECK,
    HSTATE_RESOLVE_PATH,
    HSTATE_HANDLE_METHOD,
    HSTATE_SET_HEADERS,
    HSTATE_SEND_HEADERS,
    HSTATE_SEND_BODY,
    HSTATE_COMPLETE,
    HSTATE_ERROR
};

enum CgiState {
	CGI_STATE_INIT,
	CGI_STATE_FORK,
	CGI_STATE_OPEN_TMP,
	CGI_STATE_READ_OUTPUT,
	CGI_STATE_WAIT_CHILD,
	CGI_STATE_PROCESS_RESULT,
	CGI_STATE_CLEANUP,
	CGI_STATE_DONE,
	CGI_STATE_ERROR
};


class HttpClient;

class Response
{
    private:
		std::string			_status;
	    std::string			_headers;
	    std::string			_body;

	    std::string			_filePath;
	    std::string			_contentType;
		int					_file_path_fd;

	    size_t				_bytesToSend;
	    size_t				_bytesSent;

	    char				_buffer[MAX_SEND];

		/// STATE MACHINE: Send File in CHUNK
		SendState			_sendState;
		ssize_t				_bufferLen;
		ssize_t				_bufferSent;

		/// STATE MACHINE: RESPONSE HANDLER
		HandlerState		_handlerState;
		std::string			_headerBuffer;
		size_t				_headerSent;

		/// STATE MACHINE: 
		CgiState			_cgiState;
    	int					_cgi_pipe_fd;
    	int					_cgi_tmp_fd;
    	std::string			_cgi_tmp_filename;
    	pid_t				_cgi_pid;
    	std::vector<char*>	_cgi_envp;
    	bool				_cgi_envp_built;
    	int					_cgi_child_status;

    public:
		Response();
	    ~Response();

	    void response_handler();
	    bool sendResponseChunk(bool sendHeader);

	    bool handleGetRequest();
	    bool handlePostRequest();
		bool handleUploadRequest();
	    void handleDeleteRequest();
	    void handleLoginRequest();

	    void handleFileRequest();
	    void handleDirectoryListing();
	    bool handleCGIRequest();
		void reset();

	    void setStatus(int code);
	    void setHeaders();
	    void resolveFilePath();
		int error_page(std::string error_code);

		// CGI
		void cgiInit();
		void cgiFork();
		void cgiReadOutput();
		void cgiWaitChild();
		void cgiProcessResult();
		void cgiCleanup();
		pid_t	getpid() {return _cgi_pid;};
		void  resetpid() {_cgi_pid = -1;};
		void print_trace_http();

	    static ServerConfig *findMatchingServer(std::vector<ServerConfig> &servers, Request &request);
	    static Route *findMatchingRoute(ServerConfig &server, std::string &path);

	    HttpClient *_client;
	
};


#endif