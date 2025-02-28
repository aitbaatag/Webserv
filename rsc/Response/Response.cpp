#include "../../Includes/Http_Req_Res/Response.hpp"

Response::Response() {}

Response::~Response() {}

void Response::setStatus(int code) {
    if (code == 200) {
        _status = "200 OK";
    }
    else if (code == 404) {
        _status = "404 Not Found";
    }
    else if (code == 500) {
        _status = "500 Internal Server Error";
    }
    else if (code == 405) {
        _status = "405 Method Not Allowed";
    }
    else if (code == 415) {
        _status = "415 Unsupported Media Type";
    }
    else if (code == 505) {
        _status = "505 HTTP Version Not Supported";
    }
    else {
        _status = "400 Bad Request";
    }
}

void Response::setHeaders() {
    if (_filePath.find(".html") != std::string::npos) {
        _headers = "Content-Type: text/html\r\n";
    }
    else if (_filePath.find(".css") != std::string::npos) {
        _headers = "Content-Type: text/css\r\n";
    }
    else if (_filePath.find(".js") != std::string::npos) {
        _headers = "Content-Type: text/javascript\r\n";
    }
    else if (_filePath.find(".jpg") != std::string::npos) {
        _headers = "Content-Type: image/jpeg\r\n";
    }
    else if (_filePath.find(".png") != std::string::npos) {
        _headers = "Content-Type: image/png\r\n";
    }
    else if (_filePath.find(".gif") != std::string::npos) {
        _headers = "Content-Type: image/gif\r\n";
    }
    else {
        _headers = "Content-Type: text/plain\r\n";
    }
    std::ostringstream oss;
    oss << _body.size();
    _headers += "Content-Length: " + oss.str() + "\r\n";

    _headers += "\r\n";
}

void Response::setBody() {
    std::ifstream file(_filePath.c_str(), std::ios::in | std::ios::binary);
    if (file.is_open()) {
        std::ostringstream oss;
        oss << file.rdbuf();  // Read file into a stringstream
        _body = oss.str();  // Set the response body as the file content
        file.close();
    }
    else {
        setStatus(404);
        _filePath = "www/404.html";
        setBody();
    }
}


void Response::handleFileRequest() {
    struct stat fileStat;
    if (_filePath.find(".") == std::string::npos) {
        _filePath += ".html";
    }
    if (stat(_filePath.c_str(), &fileStat) == 0) {
        setStatus(200);
        setBody();
    }
    else {
        setStatus(404);
        _filePath = "www/404.html";
        setBody();
    }
}

void Response::generateResponse(HttpClient &client, int fd) {
    if (client.Srequest.error_status == 0) {
        if (client.Srequest.method == "GET") {
            std::cout << "GET request received" << std::endl;
            if (client.Srequest.path == "/") {
                _filePath = "www/index.html";
            }
            else {
                _filePath = "www" + client.Srequest.path;
            }
            handleFileRequest();
        }
        else if (client.Srequest.method == "POST") {
            std::cout << "POST request received" << std::endl;

            _filePath = "www" + client.Srequest.path;
            handleFileRequest();
        }
        // else if (client.Srequest.method == "DELETE") {
        //     std::cout << "DELETE request received" << std::endl;
        //     setStatus(405);
        //     _filePath = "www/www/405.html";
        //     setBody();
        // }
    }
    else {
        setStatus(client.Srequest.error_status);
    }

    setHeaders();

    std::string fullResponse = "HTTP/1.1 " + _status + "\r\n" + _headers + _body;

    ssize_t bytes_sent = send(fd, fullResponse.c_str(), fullResponse.size(), 0);
    
    if (bytes_sent < 0) {
        std::cerr << "Error sending response to client FD " << fd << std::endl;
    }
    else {
        std::cout << "Sent response to client FD " << fd << std::endl;
        client.set_response_status(Complete);
    }
}
