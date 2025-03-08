#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>  // For building responses
#include <sys/types.h>  
#include <sys/stat.h>  // For checking file existence
#include <unistd.h>    // For access()
#include "../../Includes/http_client/http_client.hpp"


class Response {
    private:
        std::string _status;
        std::string _headers;
        std::string _body;
        std::string _filePath;

        void setStatus(int code);
        void setHeaders();
        void setBody();
        void handleFileRequest();
        void handleCGIRequest();

    public:
        Response();
        ~Response();

    void generateResponse(HttpClient &client, int fd);
};

#endif
