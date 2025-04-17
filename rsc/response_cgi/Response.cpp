#include "../../Includes/Http_Req_Res/Response.hpp"

Response::Response() : _bytesToSend(0), _bytesSent(0), _headersSent(false) {}

Response::~Response() {
    if (_fileStream.is_open()) {
        _fileStream.close();
    }
}

void Response::setStatus(int code) {
    std::map<int, std::string> statusMessages;
    statusMessages[200] = "200 OK";
    statusMessages[201] = "201 Created";
    statusMessages[204] = "204 No Content";
    statusMessages[301] = "301 Moved Permanently";
    statusMessages[302] = "302 Found";
    statusMessages[400] = "400 Bad Request";
    statusMessages[401] = "401 Unauthorized";
    statusMessages[403] = "403 Forbidden";
    statusMessages[404] = "404 Not Found";
    statusMessages[405] = "405 Method Not Allowed";
    statusMessages[408] = "408 Request Timeout";
    statusMessages[413] = "413 Payload Too Large";
    statusMessages[415] = "415 Unsupported Media Type";
    statusMessages[500] = "500 Internal Server Error";
    statusMessages[501] = "501 Not Implemented";
    statusMessages[505] = "505 HTTP Version Not Supported";

    if (statusMessages.find(code) != statusMessages.end()) {
        _status = statusMessages[code];
    } else {
        _status = "500 Internal Server Error";
    }
}

void Response::setHeaders() {
    _headers = "";
    std::string contentType = "text/plain";

    if (_body.find("<html>") != std::string::npos) {
        contentType = "text/html";
    }
    else {
        std::map<std::string, std::string> mimeTypes;
        
        mimeTypes[".html"] = "text/html";
        mimeTypes[".htm"] = "text/html";
        mimeTypes[".css"] = "text/css";
        mimeTypes[".js"] = "text/javascript";
        mimeTypes[".json"] = "application/json";
        mimeTypes[".jpg"] = "image/jpeg";
        mimeTypes[".jpeg"] = "image/jpeg";
        mimeTypes[".png"] = "image/png";
        mimeTypes[".gif"] = "image/gif";
        mimeTypes[".svg"] = "image/svg+xml";
        mimeTypes[".ico"] = "image/x-icon";
        mimeTypes[".pdf"] = "application/pdf";
        mimeTypes[".zip"] = "application/zip";
        mimeTypes[".txt"] = "text/plain";
        mimeTypes[".py"] = "text/html";
        mimeTypes[".php"] = "text/html";

        size_t dotPos = _filePath.find_last_of(".");
        if (dotPos != std::string::npos) {
            std::string extension = _filePath.substr(dotPos);
            if (mimeTypes.find(extension) != mimeTypes.end()) {
                contentType = mimeTypes[extension];
            }
        }
    }
    
    _headers += "Content-Type: " + contentType + "\r\n";
    
    char buffer[100];
    time_t now = time(0);
    struct tm *timeinfo = gmtime(&now);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    _headers += "Date: " + std::string(buffer) + "\r\n";
    _headers += "Server: WebServ/1.0\r\n";
    _headers += "Content-Length: " + std::to_string(_bytesToSend) + "\r\n";
    _headers += "Connection: close\r\n";
    _headers += "\r\n";
}

void Response::handleDirectoryListing(const std::string& path, const std::string& uri) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        setStatus(403);
        _body = "<html><body><h1>403 Forbidden</h1><p>Cannot access directory.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    std::ostringstream html;
    html << "<html><head><title>Directory Listing: " << uri << "</title>";
    html << "<style>body{font-family:Arial,sans-serif;margin:20px;} h1{color:#333;} "
         << "table{border-collapse:collapse;width:100%;} "
         << "th,td{padding:8px;text-align:left;border-bottom:1px solid #ddd;} "
         << "tr:hover{background-color:#f5f5f5;} "
         << "a{text-decoration:none;color:#0066cc;} "
         << "a:hover{text-decoration:underline;}</style></head>";
    html << "<body><h1>Directory Listing: " << uri << "</h1>";
    html << "<table><tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>";
    
    // Add parent directory link if not in root
    if (uri != "/") {
        html << "<tr><td><a href=\"..\">..</a></td><td>-</td><td>-</td></tr>";
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue; // Skip . and ..
        }
        
        std::string entryPath = path + "/" + entry->d_name;
        struct stat entryStat;
        
        if (stat(entryPath.c_str(), &entryStat) == 0) {
            std::string linkPrefix = (uri == "/" ? "" : uri);
            html << "<tr><td><a href=\"" << linkPrefix << (linkPrefix.empty() ? "" : "/") << entry->d_name << "\">" << entry->d_name << "</a></td>";
            
            if (S_ISDIR(entryStat.st_mode)) {
                html << "<td>Directory</td>";
            }
            else {
                // Display file size
                if (entryStat.st_size < 1024) {
                    html << "<td>" << entryStat.st_size << " B</td>";
                } else if (entryStat.st_size < 1024 * 1024) {
                    html << "<td>" << (entryStat.st_size / 1024) << " KB</td>";
                } else {
                    html << "<td>" << (entryStat.st_size / (1024 * 1024)) << " MB</td>";
                }
            }
            
            // Display last modified time
            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&entryStat.st_mtime));
            html << "<td>" << timeStr << "</td></tr>";
        }
    }
    
    html << "</table></body></html>";
    closedir(dir);
    
    _body = html.str();
    _bytesToSend = _body.size();
    setStatus(200);
}


void Response::handleFileRequest(const ServerConfig& server, const Route& route) {
    struct stat fileStat;
    
    if (stat(_filePath.c_str(), &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode)) {
            if (route.directory_listing) {
                handleDirectoryListing(_filePath, route.path);
                return;
            } 
            else if (!route.default_file.empty()) {
                std::string defaultFilePath = _filePath + "/" + route.default_file;
                if (stat(defaultFilePath.c_str(), &fileStat) == 0 && !S_ISDIR(fileStat.st_mode)) {
                    _filePath = defaultFilePath;
                }
                else {
                    setStatus(403);
                    _body = "<html><body><h1>403 Forbidden</h1><p>Directory listing not allowed.</p></body></html>";
                    _bytesToSend = _body.size();
                    return;
                }
            }
            else {
                setStatus(403);
                _body = "<html><body><h1>403 Forbidden</h1><p>Directory access denied.</p></body></html>";
                _bytesToSend = _body.size();
                return;
            }
        }
        
        _fileStream.open(_filePath.c_str(), std::ios::in | std::ios::binary);
        if (_fileStream.is_open()) {
            // Get file size
            _fileStream.seekg(0, std::ios::end);
            _bytesToSend = _fileStream.tellg();
            _fileStream.seekg(0, std::ios::beg);
            
            setStatus(200);
            _body = "";
        }
        else {
            setStatus(403);
            _body = "<html><body><h1>403 Forbidden</h1><p>Cannot access file.</p></body></html>";
            _bytesToSend = _body.size();
        }
    }
    else {
        setStatus(404);
        if (server.error_page.find("404") != server.error_page.end()) {
            std::string errorPagePath = server.error_page.at("404");
            struct stat fileStat;
            if (stat(errorPagePath.c_str(), &fileStat) == 0 && !S_ISDIR(fileStat.st_mode)) {
                _fileStream.open(errorPagePath.c_str(), std::ios::in | std::ios::binary);
                if (_fileStream.is_open()) {
                    _fileStream.seekg(0, std::ios::end);
                    _bytesToSend = _fileStream.tellg();
                    _fileStream.seekg(0, std::ios::beg);
                    return;
                }
            }
        }
        _body = "<html><body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>";
        _bytesToSend = _body.size();
    }
}

void Response::handleDeleteRequest(const Request& request, const Route& route) {
    struct stat fileStat;
    if (stat(_filePath.c_str(), &fileStat) != 0) {
        setStatus(404);
        _body = "<html><body><h1>404 Not Found</h1><p>The resource does not exist.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    if (S_ISDIR(fileStat.st_mode)) {
        setStatus(403);
        _body = "<html><body><h1>403 Forbidden</h1><p>Cannot delete a directory.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    if (unlink(_filePath.c_str()) != 0) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to delete the resource.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    setStatus(200);
    _body = "<html><body><h1>200 OK</h1><p>Resource deleted successfully.</p></body></html>";
    _bytesToSend = _body.size();
}

const ServerConfig& Response::findMatchingServer(const std::vector<ServerConfig>& servers, const Request& request) {
    for (size_t i = 0; i < servers.size(); i++) {
        const ServerConfig& server = servers[i];

        for (size_t j = 0; j < server.server_names.size(); j++) {
            if (server.server_names[j] == request.headers.at("Host")) {
                return server;
            }
        }
    }

    return servers[0];
}

const Route& Response::findMatchingRoute(const ServerConfig& server, const std::string& path) {
    std::string bestMatch = "";
    size_t bestMatchIndex = 0;

    for (size_t i = 0; i < server.routes.size(); i++) {
        if (server.routes[i].path == path) {
            return server.routes[i];
        }
    }

    for (size_t i = 0; i < server.routes.size(); i++) {
        const std::string& routePath = server.routes[i].path;
        
        if (path.compare(0, routePath.length(), routePath) == 0) {
            if (routePath.length() > bestMatch.length()) {
                bestMatch = routePath;
                bestMatchIndex = i;
            }
        }
    }

    if (!bestMatch.empty()) {
        return server.routes[bestMatchIndex];
    }

    for (size_t i = 0; i < server.routes.size(); i++) {
        if (server.routes[i].path == "/") {
            return server.routes[i];
        }
    }

    // If no default route exists, return the first route (should not happen with valid config)
    return server.routes[0];
}

void Response::response_handler(HttpClient &client, int fd, const std::vector<ServerConfig>& servers) {
    if (!_headersSent) {
        const ServerConfig& server = findMatchingServer(servers, client.Srequest);
        const Route& route = findMatchingRoute(server, client.Srequest.path);
        
        bool methodAllowed = false;
        for (size_t i = 0; i < route.accepted_methods.size(); i++) {
            if (route.accepted_methods[i] == client.Srequest.method) {
                methodAllowed = true;
                break;
            }
        }
        
        if (!methodAllowed) {
            setStatus(405);
            _body = "<html><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed for this resource.</p></body></html>";
            _bytesToSend = _body.size();
        }
        else if (client.Srequest.error_status != 0) {
            setStatus(client.Srequest.error_status);
            _body = "<html><body><h1>" + _status + "</h1><p>An error occurred processing your request.</p></body></html>";
            _bytesToSend = _body.size();
        }
        else if (!route.redirect.empty()) {
            // Handle redirection
            setStatus(301); // Or 302 based on your configuration
            _headers = "Location: " + route.redirect + "\r\n";
            _body = "<html><body><h1>301 Moved Permanently</h1><p>Resource has been moved to <a href=\"" + route.redirect + "\">" + route.redirect + "</a>.</p></body></html>";
            _bytesToSend = _body.size();
        }
        else {
            if (client.Srequest.path == "/") {
                _filePath = "." + route.root_dir + "/" + route.default_file;
            }
            else {
                _filePath = "." + route.root_dir + client.Srequest.path;
            }
        
            std::cout << "File path: " << _filePath << std::endl;
            
            if (client.Srequest.method == "GET") {
                size_t dotPos = _filePath.find_last_of(".");
                if (dotPos != std::string::npos) {
                    std::string extension = _filePath.substr(dotPos);
                    if (route.cgi_extension.find(extension) != route.cgi_extension.end()) {
                        std::cout << "Handling CGI request for file: " << _filePath << std::endl;
                        handleCGIRequest(client.Srequest, route);
                    }
                    else {
                        handleFileRequest(server, route);
                    }
                }
                else {
                    handleFileRequest(server, route);
                }
            }
            else if (client.Srequest.method == "POST") {
                size_t dotPos = _filePath.find_last_of(".");
                if (dotPos != std::string::npos) {
                    std::string extension = _filePath.substr(dotPos);
                    if (route.cgi_extension.find(extension) != route.cgi_extension.end()) {
                        std::cout << "Handling CGI request for file: " << _filePath << std::endl;
                        handleCGIRequest(client.Srequest, route);
                    }
                    // else if (!route.upload_dir.empty()) {
                    //     // Handle file upload
                    //     std::string uploadPath = route.upload_dir + "/" + client.Srequest.headers.at("Filename");
                    //     std::ofstream uploadFile(uploadPath, std::ios::binary);
                    //     if (uploadFile.is_open()) {
                    //         uploadFile.write(client.Srequest.body.c_str(), client.Srequest.body.size());
                    //         uploadFile.close();
                    //         setStatus(201);
                    //         _body = "<html><body><h1>201 Created</h1><p>File uploaded successfully.</p></body></html>";
                    //     }
                    //     else {
                    //         setStatus(500);
                    //         _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to save uploaded file.</p></body></html>";
                    //     }
                    //     _bytesToSend = _body.size();
                    // }
                    else {
                        setStatus(405);
                        _body = "<html><body><h1>405 Method Not Allowed</h1><p>POST method not allowed for this resource.</p></body></html>";
                        _bytesToSend = _body.size();
                    }
                }
                else {
                    setStatus(405);
                    _body = "<html><body><h1>405 Method Not Allowed</h1><p>POST method not allowed for this resource.</p></body></html>";
                    _bytesToSend = _body.size();
                }
            }
            else if (client.Srequest.method == "DELETE") {
                handleDeleteRequest(client.Srequest, route);
            }
            else {
                setStatus(501);
                _body = "<html><body><h1>501 Not Implemented</h1><p>The requested method is not supported.</p></body></html>";
                _bytesToSend = _body.size();
            }
        }
        
        setHeaders();
        
        std::string responseHeader = "HTTP/1.1 " + _status + "\r\n" + _headers;
        
        ssize_t bytesSent = send(fd, responseHeader.c_str(), responseHeader.size(), 0);
        if (bytesSent < 0) {
            std::cerr << "Error sending response headers to client FD " << fd << std::endl;
            return;
        }
        else {
            std::cout << "Sent response headers to client FD " << fd << std::endl;
        }
        
        _headersSent = true;
        _bytesSent = 0;
    }
    
    bool completed = sendResponseChunk(fd);
    
    if (completed) {
        std::cout << "Sent full response to client FD " << fd << std::endl;
        client.set_response_status(Complete);
        _headersSent = false;
        _bytesSent = 0;
        _bytesToSend = 0;
    }
}

bool Response::sendResponseChunk(int fd) {
    if (_fileStream.is_open()) {
        // Send file content in chunks
        _fileStream.read(_buffer, BUFFER_SIZE);
        size_t bytesRead = _fileStream.gcount();
        
        if (bytesRead > 0) {
            ssize_t bytesSent = send(fd, _buffer, bytesRead, 0);
            if (bytesSent < 0) {
                std::cerr << "Error sending file chunk to client FD " << fd << std::endl;
                _fileStream.close();
                return true;
            }
            
            _bytesSent += bytesSent;
            
            if (_bytesSent >= _bytesToSend || _fileStream.eof()) {
                _fileStream.close();
                return true;
            }
            
            return false;
        }
        else {
            _fileStream.close();
            return true;
        }
    }
    else if (_bytesToSend > 0) {
        // Send body content in one go (for small responses)
        ssize_t bytesSent = send(fd, _body.c_str() + _bytesSent, _bytesToSend - _bytesSent, 0);
        if (bytesSent < 0) {
            std::cerr << "Error sending response body to client FD " << fd << std::endl;
            return true;
        }
        
        _bytesSent += bytesSent;
        
        if (_bytesSent >= _bytesToSend) {
            return true;
        }
        
        return false;
    }
    else {
        return true;
    }
}