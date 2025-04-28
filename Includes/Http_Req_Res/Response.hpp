#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "../libraries/Libraries.hpp"
#include "../Config/Config.hpp"
#include "../Http_Req_Res/Request_Struct.hpp"


class HttpClient;

class Response
{
    private:
		std::string _status;
	    std::string _headers;
	    std::string _body;

	    std::string _filePath;
	    std::string _contentType;
	    std::ifstream _fileStream;

	    size_t _bytesToSend;
	    size_t _bytesSent;
		pid_t _pid;
	    bool _headersSent;
	    char _buffer[MAX_SEND];

    public:
		Response();
	    ~Response();

	    void response_handler();
	    bool sendResponseChunk();

	    void handleGetRequest();
	    void handlePostRequest();
	    void handleDeleteRequest();
	    void handleLoginRequest();

	    void handleFileRequest();
	    void handleDirectoryListing();
	    void handleCGIRequest();

	    void setStatus(int code);
	    void setHeaders();
	    void resolveFilePath();
		int error_page(std::string error_code);

	    static ServerConfig *findMatchingServer(std::vector<ServerConfig> &servers, Request &request);
	    static Route *findMatchingRoute(ServerConfig &server, std::string &path);

	    HttpClient *_client;

	
};


#endif