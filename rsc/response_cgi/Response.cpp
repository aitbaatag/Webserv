#include "../../Includes/Http_Req_Res/Response.hpp"
#include "../../Includes/http_client/http_client.hpp"

Response::Response() {
    _fileStream = NULL;
    _status = "";
    _headers = "";
    _body = "";
    _filePath = "";
    _contentType = "text/plain";
    _bytesToSend = 0;
    _bytesSent = 0;
    _headersSent = false;
}

Response::~Response() {
    if (_fileStream) {
        if (_fileStream->is_open()) {
            _fileStream->close();
        }
        delete _fileStream;
        _fileStream = NULL;
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
    if (_body.find("html>") != std::string::npos) {
        _contentType = "text/html";
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
        size_t dotPos = _filePath.find_last_of(".");

        if (dotPos != std::string::npos) {
            std::string extension = _filePath.substr(dotPos);
            if (mimeTypes.find(extension) != mimeTypes.end()) {
                _contentType = mimeTypes[extension];
            }
        }
    }
    
    _headers += "Content-Type: " + _contentType + "\r\n";
    
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

std::string cleanSlashes(const std::string &str)
{
	std::string result;
	bool lastWasSlash = false;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == '/') {
			if (!lastWasSlash) {
				result += str[i];
				lastWasSlash = true;
			}
		}
		else {
			result += str[i];
			lastWasSlash = false;
		}
	}
	return result;
}

void Response::handleDirectoryListing(const std::string& path, const std::string& uri, std::string originalPath) {
    DIR* dir = opendir(path.c_str());
    
    if (!dir) {
        setStatus(403);
        _body = "<html><body><h1>403 Forbidden</h1><p>Cannot access directory.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    std::ostringstream html;

    html << "<!DOCTYPE html><html lang=\"en\"><head>";
    html << "<meta charset=\"UTF-8\">";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html << "<title>Directory Listing: " << uri << "</title>";
    html << "<link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap\" rel=\"stylesheet\">";
    html << "<link rel=\"stylesheet\" href=\"/style.css\">";
    html << "<style>";
    html << ".directory-container { max-width: 900px; margin: 2rem auto; background: var(--container-bg); border-radius: 16px; padding: 2.5rem; box-shadow: 0 8px 32px var(--shadow-color); border: 1px solid var(--border-color); backdrop-filter: blur(12px); }";
    html << ".files-table { width: 100%; border-collapse: collapse; margin-bottom: 2rem; }";
    html << ".files-table th, .files-table td { padding: 1rem; text-align: left; border-bottom: 1px solid var(--border-color); color: var(--text-color); }";
    html << ".files-table th { font-weight: 600; color: var(--primary-color); }";
    html << ".files-table tr:hover { background: rgba(255, 255, 255, 0.05); }";
    html << "a { color: white; text-decoration: none; }";
    html << "</style>";
    html << "</head><body>";
    html << "<div class=\"wrapperAll container\">";
    html << "<div class=\"titleSection\">";
    html << "<h1>Directory Listing</h1>";
    html << "<p>Browse the files in the directory</p>";
    html << "</div>";
    html << "<div class=\"directory-container\">";
    html << "<table class=\"files-table\">";
    html << "<thead><tr><th>Name</th><th>Type</th><th>Size</th><th>Last Modified</th></tr></thead>";
    html << "<tbody>";

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue;
        }
        
        std::string entryPath = path + "/" + entry->d_name;
        struct stat entryStat;
        
        if (stat(entryPath.c_str(), &entryStat) == 0) {
            html << "<tr>";
            std::string path = cleanSlashes(originalPath + "/" + entry->d_name);
            html << "<td><a href=\"" << path << "\">" << entry->d_name << "</a></td>";

            if (S_ISDIR(entryStat.st_mode)) {
                html << "<td>Directory</td>";
                html << "<td>-</td>";
            } else {
                html << "<td>File</td>";
                // Format file size in human-readable format
                std::string sizeStr;
                off_t size = entryStat.st_size;
                if (size >= 1024 * 1024) {
                    sizeStr = std::to_string(size / (1024 * 1024)) + " MB";
                } else if (size >= 1024) {
                    sizeStr = std::to_string(size / 1024) + " KB";
                } else {
                    sizeStr = std::to_string(size) + " B";
                }
                html << "<td>" << sizeStr << "</td>";
            }

            // Format last modified time
            char timeBuf[100];
            struct tm* timeinfo = localtime(&entryStat.st_mtime);
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
            html << "<td>" << timeBuf << "</td>";
            
            html << "</tr>";
        }
    }

    html << "</tbody></table>";
    html << "<div class=\"button-group\">";
    html << "<button class=\"button back\" onclick=\"window.history.back()\">Go Back</button>";
    html << "</div>";
    html << "</div>";
    html << "<footer><p>© 2025 Webserv - Modern HTTP Server</p></footer>";
    html << "</div>";
    html << "</body></html>";

    closedir(dir);
    
    _body = html.str();
    _bytesToSend = _body.size();
    setStatus(200);
}

void Response::handleFileRequest(const ServerConfig& server, const Route& route, std::string originalPath) {
    struct stat fileStat;
    
    if (stat(_filePath.c_str(), &fileStat) == 0) {
        std::cout << "File exists: " << _filePath << std::endl;
        if (S_ISDIR(fileStat.st_mode)) {
            if (route.directory_listing) {
                handleDirectoryListing(_filePath, route.path, originalPath);
                return;
            }
            else {
                setStatus(403);
                _body = "<html><body><h1>403 Forbidden</h1><p>Directory access denied or Directory listing not allowed.</p></body></html>";
                _bytesToSend = _body.size();
                return;
            }
        }
        
        _fileStream->open(_filePath.c_str(), std::ios::in | std::ios::binary);
        if (_fileStream->is_open()) {
            _fileStream->seekg(0, std::ios::end);
            _bytesToSend = _fileStream->tellg();
            _fileStream->seekg(0, std::ios::beg);
            setStatus(200);
        }
        else {
            setStatus(403);
            _body = "<html><body><h1>403 Forbidden</h1><p>Cannot access file.</p></body></html>";
            _bytesToSend = _body.size();
        }
    }
    else {
        std::cout << "File does not exist: " << _filePath << std::endl;
        setStatus(404);
        if (server.error_page.find("404") != server.error_page.end()) {
            std::string errorPagePath = "." + server.error_page.at("404");
            struct stat fileStat;

            if (stat(errorPagePath.c_str(), &fileStat) == 0 && !S_ISDIR(fileStat.st_mode)) {
                _fileStream->open(errorPagePath.c_str(), std::ios::in | std::ios::binary);
                if (_fileStream->is_open()) {
                    _fileStream->seekg(0, std::ios::end);
                    _bytesToSend = _fileStream->tellg();
                    _fileStream->seekg(0, std::ios::beg);
                    _contentType = "text/html";
                    return;
                }
            }
        }
        _body = "<html><body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>";
        _bytesToSend = _body.size();
    }
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
    return server.routes[0];
}

void Response::resolveFilePath(std::string& request_path, const Route& route) {
    if (request_path == route.path) {
        _filePath = "." + route.root_dir + "/" + route.default_file;
    } 
    else {
        if (route.path != "/") {
            std::string path = request_path;
            size_t pos = path.find(route.path);
            if (pos != std::string::npos) {
                path.erase(pos, route.path.length());
                request_path = path;
            }
        }
        _filePath = "." + route.root_dir + request_path;
    }
}

void Response::handleGetRequest(const Request& request, const Route& route, const ServerConfig& server) {
    std::cout << "GET request for file: " << _filePath << std::endl;
    size_t dotPos = _filePath.find_last_of(".");
    
    if (dotPos != std::string::npos) {
        std::string extension = _filePath.substr(dotPos);
        if (route.cgi_extension.find(extension) != route.cgi_extension.end()) {
            std::cout << "GET cgi request for file: " << _filePath << std::endl;
            handleCGIRequest(request, route);
        }
        else {
            handleFileRequest(server, route, request.path);
        }
    }
    
}

void Response::handlePostRequest(Request& request, const Route& route) {
    std::cout << "POST request for file: " << _filePath << std::endl;
    size_t dotPos = _filePath.find_last_of(".");
    
    if (dotPos != std::string::npos) {
        std::string extension = _filePath.substr(dotPos);
        if (route.cgi_extension.find(extension) != route.cgi_extension.end()) {
            std::cout << "POST cgi request for file: " << _filePath << std::endl;
            handleCGIRequest(request, route);
        }
        else if (!route.upload_dir.empty()) {
            std::string FileName = request.headers.at("X-File-Name").c_str();
            if (!FileName.empty()) {
                setStatus(201);
                rename(request.filename.c_str(), ("." + route.upload_dir + "/" + FileName).c_str());
                _body = "<html><body><h1>201 Created</h1><p>File uploaded successfully.</p></body></html>";
                request.fileStream.close();
            }
            else {
                unlink(request.filename.c_str());
                setStatus(500);
                _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to save uploaded file.</p></body></html>";
                request.fileStream.close();
            }
            std::cout << "uploaded successfully: " << request.filename << std::endl;
            _bytesToSend = _body.size();
        }
        else {
            setStatus(403);
            _body = "<html><body><h1>403 Forbidden</h1><p></p></body></html>";
            _bytesToSend = _body.size();
        }
    }
}

void Response::handleDeleteRequest(const Request& request, const Route& route) {
    std::cout << "DELETE request for file: " << _filePath << std::endl;
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

void Response::response_handler(HttpClient &client, int fd, const std::vector<ServerConfig>& servers) {
    if (!_headersSent) {
        if (client.Srequest.error_status != 0) {
            setStatus(client.Srequest.error_status);
            _body = "<html><body><h1>" + _status + "</h1><p>An error occurred processing your request.</p></body></html>";
            _bytesToSend = _body.size();
        }
        else {
            const ServerConfig& server = findMatchingServer(servers, client.Srequest);
            const Route& route = findMatchingRoute(server, client.Srequest.path);
            // std::cout << "Route path: " << route.path << std::endl;
            
            if (!route.redirect.empty()) {
                setStatus(301);
                _headers = "Location: " + route.redirect + "\r\n";
                _body = "";
                _bytesToSend = 0;
            }
            else {
                std::cout << "request path: " << client.Srequest.path << std::endl;
                std::cout << "Request method: " << client.Srequest.method << std::endl;
                resolveFilePath(client.Srequest.path, route);
                        
                if (client.Srequest.method == "GET") {
                    handleGetRequest(client.Srequest, route, server);
                }
                else if (client.Srequest.method == "POST") {
                    handlePostRequest(client.Srequest, route);
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
    
    bool completed = sendResponseChunk(fd, client);
    
    if (completed) {
        // std::cout << "Response body : " << _body << std::endl;
        std::cout << "Sent full response to client FD " << fd << std::endl;
        client.set_response_status(Complete);
    }
}

bool Response::sendResponseChunk(int fd, HttpClient &client) {
    if (_fileStream->is_open()) {
        _fileStream->read(_buffer, BUFFER_SIZE);
        size_t bytesRead = _fileStream->gcount();
        
        if (bytesRead > 0) {
            ssize_t bytesSent = send(fd, _buffer, bytesRead, 0);
            
            if (bytesSent < 0) {
                std::cerr << "Error sending file chunk to client FD " << fd << std::endl;
                _fileStream->close();
                return true;
            }
            client.time_client_ = time(NULL);
            
            _bytesSent += bytesSent;
            
            if (_bytesSent >= _bytesToSend || _fileStream->eof()) {
                _fileStream->close();
                return true;
            }
            
            return false;
        }
        else {
            _fileStream->close();
            return true;
        }
    }
    else if (_bytesToSend > 0) {
        ssize_t bytesSent = send(fd, _body.c_str() + _bytesSent, _bytesToSend - _bytesSent, 0);
        client.time_client_ = time(NULL);

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