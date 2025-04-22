#ifndef RESPONSE_HPP
#define RESPONSE_HPP


#include "../libraries/Libraries.hpp"
#include "../Config/Config.hpp"
#include "../Http_Req_Res/Request_Struct.hpp"

class HttpClient;


class Response {
private:
    std::string _status;
    std::string _headers;
    std::string _body;
    std::string _filePath;
    size_t _bytesToSend;
    size_t _bytesSent;
    bool _headersSent;
    std::ifstream *_fileStream;
    
    // Buffer for reading files in chunks
    static const size_t BUFFER_SIZE = 8192;
    char _buffer[BUFFER_SIZE];

public:
    Response();
    ~Response();

    void creatFilestream() {_fileStream = new std::ifstream();};
    void setStatus(int code);
    void setHeaders();
    void handleFileRequest(const ServerConfig& server, const Route& route, std::string originalPath);
    void handleDirectoryListing(const std::string& path, const std::string& uri, std::string originalPath);
    void handleCGIRequest(const Request& request, const Route& route);
    void handleDeleteRequest(const Request& request, const Route& route);
    void response_handler(HttpClient &client, int fd, const std::vector<ServerConfig>& servers);
    bool sendResponseChunk(int fd);
    const ServerConfig& findMatchingServer(const std::vector<ServerConfig>& servers, const Request& request);
    const Route& findMatchingRoute(const ServerConfig& server, const std::string& path);
};

#endif