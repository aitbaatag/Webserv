#include "../../Includes/Http_Req_Res/Response.hpp"
#include "../../Includes/http_client/http_client.hpp"
#include "../../Includes/server/server_socket.hpp"
#include "../../Includes/utlis/utils.hpp"

Response::Response() {
  _file_path_fd = -1;
  _status = "";
  _headers = "";
  _body = "";
  _filePath = "";
  _contentType = "text/plain";
  _bytesToSend = 0;
  _bytesSent = 0;

  // --- Send Chunk state machine initialization ---
  _sendState = SEND_IDLE;
  _bufferLen = 0;
  _bufferSent = 0;

  // --- response handler state machine initialization ---
  _handlerState = HSTATE_ERROR_CHECK;
  _headerSent = 0;

  // --- CGI state machine initialization ---
  _cgiState = CGI_STATE_INIT;
  _cgi_pipe_fd = -1;
  _cgi_tmp_fd = -1;
  _cgi_pid = -1;
  _cgi_envp.clear();
  _cgi_envp_built = false;
  _cgi_child_status = 0;
}

void Response::reset() {
  delete_file(_cgi_tmp_filename);
  close_fd(_file_path_fd);
  close_fd(_cgi_pipe_fd);
  close_fd(_cgi_tmp_fd);

  if (_cgi_envp_built) {
    for (size_t i = 0; i < _cgi_envp.size(); ++i) {
      free(_cgi_envp[i]);
      _cgi_envp[i] = NULL;
    }

    _cgi_envp.clear();
    _cgi_envp_built = false;
  }

  _status.clear();
  _headers.clear();
  _body.clear();
  _filePath.clear();
  _contentType = "text/plain";
  _bytesToSend = 0;
  _bytesSent = 0;
  _client = NULL;
  close_fd(_file_path_fd);

  // --- Send Chunk state machine initialization ---
  _sendState = SEND_IDLE;
  _bufferLen = 0;
  _bufferSent = 0;

  // --- response handler state machine initialization ---
  _handlerState = HSTATE_ERROR_CHECK;
  _headerSent = 0;

  // --- CGI state machine initialization ---
  _cgiState = CGI_STATE_INIT;
  _cgi_pipe_fd = -1;
  _cgi_tmp_fd = -1;
  _cgi_pid = -1;
  _cgi_envp.clear();
  _cgi_envp_built = false;
  _cgi_child_status = 0;
}

Response::~Response() {
  delete_file(_cgi_tmp_filename);
  close_fd(_file_path_fd);
  close_fd(_cgi_pipe_fd);
  close_fd(_cgi_tmp_fd);
  if (_cgi_envp_built) {
    for (size_t i = 0; i < _cgi_envp.size(); ++i) {
      free(_cgi_envp[i]);
      _cgi_envp[i] = NULL;
    }
    _cgi_envp.clear();
    _cgi_envp_built = false;
  }
  _client = NULL;
}

void Response::setStatus(int code) {
  std::map<int, std::string> statusMessages;

  statusMessages[200] = "200 OK";
  statusMessages[201] = "201 Created";
  statusMessages[202] = "202 Accepted";
  statusMessages[204] = "204 No Content";
  statusMessages[206] = "206 Partial Content";
  statusMessages[301] = "301 Moved Permanently";
  statusMessages[302] = "302 Found";
  statusMessages[303] = "303 See Other";
  statusMessages[307] = "307 Temporary Redirect";
  statusMessages[308] = "308 Permanent Redirect";
  statusMessages[400] = "400 Bad Request";
  statusMessages[401] = "401 Unauthorized";
  statusMessages[403] = "403 Forbidden";
  statusMessages[404] = "404 Not Found";
  statusMessages[405] = "405 Method Not Allowed";
  statusMessages[408] = "408 Request Timeout";
  statusMessages[409] = "409 Conflict";
  statusMessages[411] = "411 Length Required";
  statusMessages[413] = "413 Payload Too Large";
  statusMessages[414] = "414 URI Too Long";
  statusMessages[415] = "415 Unsupported Media Type";
  statusMessages[416] = "416 Range Not Satisfiable";
  statusMessages[417] = "417 Expectation Failed";
  statusMessages[422] = "422 Unprocessable Entity";
  statusMessages[429] = "429 Too Many Requests";
  statusMessages[431] = "431 Request Header Fields Too Large";
  statusMessages[500] = "500 Internal Server Error";
  statusMessages[501] = "501 Not Implemented";
  statusMessages[502] = "502 Bad Gateway";
  statusMessages[503] = "503 Service Unavailable";
  statusMessages[505] = "505 HTTP Version Not Supported";

  if (statusMessages.find(code) != statusMessages.end()) {
    _status = statusMessages[code];
  } else {
    _status = statusMessages[500];
  }
}

int Response::error_page(std::string error_code) {
  if (!_client)
    return 0;

  if (!_client->server_config)
    _client->server_config = &(_client->server->getServerConfig()[0]);

  setStatus(atoi(error_code.c_str()));

  if (_client->server_config->error_page.find(error_code) !=
      _client->server_config->error_page.end()) {
    std::string errorPagePath =
        "." + _client->server_config->error_page.at(error_code);
    struct stat fileStat;
    if (stat(errorPagePath.c_str(), &fileStat) == 0 &&
        !S_ISDIR(fileStat.st_mode)) {
      int fd = open(errorPagePath.c_str(), O_RDONLY);
      if (fd != -1) {
        close_fd(_file_path_fd);
        _file_path_fd = fd;
        _bytesToSend = fileStat.st_size;
        _contentType = "text/html";
        return 1;
      }
    }
  }

  return 0;
}

void Response::setHeaders() {
  if (_body.find("html>") != std::string::npos) {
    _contentType = "text/html";
  } else {
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
    mimeTypes[".mp4"] = "video/mp4";
    mimeTypes[".webm"] = "video/webm";
    mimeTypes[".mp3"] = "audio/mpeg";
    mimeTypes[".wav"] = "audio/wav";
    mimeTypes[".ogg"] = "audio/ogg";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".zip"] = "application/zip";
    mimeTypes[".tar"] = "application/x-tar";
    mimeTypes[".gz"] = "application/gzip";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".xml"] = "application/xml";
    size_t dotPos = _filePath.find_last_of(".");

    if (dotPos != std::string::npos) {
      std::string extension = _filePath.substr(dotPos);
      if (mimeTypes.find(extension) != mimeTypes.end()) {
        _contentType = mimeTypes[extension];
      }
    }
  }
  char buffer[100];
  time_t now = time(0);
  struct tm *timeinfo = gmtime(&now);
  strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
  if (_body.size() > 0)
    _bytesToSend = _body.size();

  _headers += "Content-Type: " + _contentType + "\r\n";
  _headers += "Date: " + std::string(buffer) + "\r\n";
  _headers += "Server: WebServ/1.0\r\n";
  _headers += "Content-Length: " + to_string(_bytesToSend) + "\r\n";
  if (_client->get_request_status() == Failed)
    _headers += "Connection: closed\r\n";
  _headers += "\r\n";
}

std::string cleanSlashes(const std::string &str) {
  std::string result;
  bool lastWasSlash = false;

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '/') {
      if (!lastWasSlash) {
        result += str[i];
        lastWasSlash = true;
      }
    } else {
      result += str[i];
      lastWasSlash = false;
    }
  }
  return result;
}

void Response::handleDirectoryListing() {
  std::ostringstream html;
  DIR *dir = opendir(_filePath.c_str());

  if (!dir) {
    if (error_page("403") == 0) {
      _body = "<html><body><h1>403 Forbidden</h1><p>Cannot access "
              "directory.</p></body></html>";
      return;
    }
  }

  html << "<!DOCTYPE html><html lang=\"en\"><head>";
  html << "<meta charset=\"UTF-8\">";
  html << "<meta name=\"viewport\" content=\"width=device-width, "
          "initial-scale=1.0\">";
  html << "<title>Directory Listing: " << _client->route->path << "</title>";
  html << "<link "
          "href=\"https://fonts.googleapis.com/"
          "css2?family=Inter:wght@400;500;600;700&display=swap\" "
          "rel=\"stylesheet\">";
  html << "<link rel=\"stylesheet\" href=\"/style.css\">";
  html << "<style>";
  html << ".directory-container { max-width: 900px; margin: 2rem auto; "
          "background: var(--container-bg); border-radius: 16px; padding: "
          "2.5rem; box-shadow: 0 8px 32px var(--shadow-color); border: 1px "
          "solid var(--border-color); backdrop-filter: blur(12px); }";
  html << ".files-table { width: 100%; border-collapse: collapse; "
          "margin-bottom: 2rem; }";
  html << ".files-table th, .files-table td { padding: 1rem; text-align: left; "
          "border-bottom: 1px solid var(--border-color); color: "
          "var(--text-color); }";
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
  html << "<thead><tr><th>Name</th><th>Type</th><th>Size</th><th>Last "
          "Modified</th></tr></thead>";
  html << "<tbody>";

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (std::string(entry->d_name) == "." ||
        std::string(entry->d_name) == "..") {
      continue;
    }
    std::string entryPath = _filePath + "/" + entry->d_name;
    struct stat entryStat;

    if (stat(entryPath.c_str(), &entryStat) == 0) {
      html << "<tr>";
      std::string path =
          cleanSlashes(_client->Srequest.path + "/" + entry->d_name);
      html << "<td><a href=\"" << path << "\">" << entry->d_name << "</a></td>";

      if (S_ISDIR(entryStat.st_mode)) {
        html << "<td>Directory</td>";
        html << "<td>-</td>";
      } else {
        html << "<td>File</td>";
        std::string sizeStr;
        off_t size = entryStat.st_size;

        if (size >= 1024 * 1024) {
          sizeStr = to_string(size / (1024 * 1024)) + " MB";
        } else if (size >= 1024) {
          sizeStr = to_string(size / 1024) + " KB";
        } else {
          sizeStr = to_string(size) + " B";
        }
        html << "<td>" << sizeStr << "</td>";
      }
      char timeBuf[100];
      struct tm *timeinfo = localtime(&entryStat.st_mtime);
      strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", timeinfo);
      html << "<td>" << timeBuf << "</td>";
      html << "</tr>";
    }
  }

  html << "</tbody></table>";
  html << "<div class=\"button-group\">";
  html << "<button class=\"button back\" onclick=\"window.history.back()\">Go "
          "Back</button>";
  html << "</div>";
  html << "</div>";
  html << "<footer><p>© 2025 Webserv - Modern HTTP Server</p></footer>";
  html << "</div>";
  html << "</body></html>";

  closedir(dir);
  _body = html.str();
  setStatus(200);
}

void Response::handleFileRequest() {
  struct stat fileStat;

  if (stat(_filePath.c_str(), &fileStat) == 0) {
    if (S_ISDIR(fileStat.st_mode)) {
      if (_client->route->directory_listing) {
        handleDirectoryListing();
        return;
      } else {
        if (error_page("403") == 0) {
          _body = "<html><body> < h1>403 Forbidden</h1 > < p>Directory access "
                  "denied or Directory listing not allowed.</p></body></html>";
        }
        return;
      }
    }

    int fd = open(_filePath.c_str(), O_RDONLY);
    if (fd != -1) {
      _file_path_fd = fd;
      _bytesToSend = fileStat.st_size;
      setStatus(200);
    } else {
      if (error_page("403") == 0) {
        _body = "<html><body> < h1>403 Forbidden</h1 > < p>Cannot access "
                "file.</p></body></html>";
      }
    }
  } else {
    if (error_page("404") == 0) {
      _body = "<html><body> < h1>404 Not Found</h1 > < p>The requested "
              "resource could not be found.</p></body></html>";
    }
  }
}

ServerConfig *Response::findMatchingServer(std::vector<ServerConfig> &servers,
                                           Request &request) {
  for (size_t i = 0; i < servers.size(); i++) {
    ServerConfig &server = servers[i];

    for (size_t j = 0; j < server.server_names.size(); j++) {
      if (server.server_names[j] == request.headers.at("Host")) {
        return &server;
      }
    }
  }

  return &servers[0];
}

Route *Response::findMatchingRoute(ServerConfig &server, std::string &path) {
  std::string bestMatch = "";
  size_t bestMatchIndex = 0;

  for (size_t i = 0; i < server.routes.size(); i++) {
    if (server.routes[i].path == path) {
      return &(server.routes[i]);
    }
  }

  for (size_t i = 0; i < server.routes.size(); i++) {
    const std::string &routePath = server.routes[i].path;

    if (path.compare(0, routePath.length(), routePath) == 0) {
      if (routePath.length() > bestMatch.length()) {
        bestMatch = routePath;
        bestMatchIndex = i;
      }
    }
  }

  if (!bestMatch.empty()) {
    return &(server.routes[bestMatchIndex]);
  }

  for (size_t i = 0; i < server.routes.size(); i++) {
    if (server.routes[i].path == "/") {
      return &(server.routes[i]);
    }
  }

  return &(server.routes[0]);
}
void Response::resolveFilePath() {
  if (_client->Srequest.path == _client->route->path) {
    _filePath =
        "." + _client->route->root_dir + "/" + _client->route->default_file;
  } else {
    if (_client->route->path != "/") {
      std::string path = _client->Srequest.path;
      size_t pos = path.find(_client->route->path);
      if (pos != std::string::npos) {
        path.erase(pos, _client->route->path.length());
        _client->Srequest.path = path;
      }
    }
    _filePath = "." + _client->route->root_dir + _client->Srequest.path;
  }
}

bool Response::handleGetRequest() {
  size_t dotPos = _filePath.find_last_of(".");

  if (dotPos != std::string::npos) {
    std::string extension = _filePath.substr(dotPos);
    if ((_client->route->cgi_extension.find(extension)) !=
        _client->route->cgi_extension.end()) {
      return (handleCGIRequest());
    } else {
      handleFileRequest();
      return true;
    }
  }
  return true;
}

std::string parseCookies(const std::string &cookieHeader,
                         const std::string &key) {
  if (cookieHeader.empty()) {
    return "";
  }

  std::istringstream stream(cookieHeader);
  std::string pair;

  while (std::getline(stream, pair, ';')) {
    size_t start = pair.find_first_not_of(" ");
    if (start != std::string::npos) {
      pair = pair.substr(start);
    }

    size_t separator = pair.find('=');
    if (separator != std::string::npos) {
      std::string cookieKey = pair.substr(0, separator);
      std::string cookieValue = pair.substr(separator + 1);

      if (cookieKey == key) {
        return cookieValue;
      }
    }
  }

  return "";
}
bool fd_getline(int fd, std::string &line) {
  line.clear();
  char c;
  ssize_t n;
  while ((n = read(fd, &c, 1)) == 1) {
    if (c == '\n')
      break;
    if (c != '\r')
      line += c;
  }

  return (!line.empty() || n == 1);
}

void Response::handleLoginRequest() {
  std::string action, username, note;
  std::string line;

  int fd = _client->Srequest.fd_file;
  lseek(fd, 0, SEEK_SET);

  while (fd_getline(fd, line)) {
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      std::string value = line.substr(colonPos + 1);

      if (key == "action")
        action = value;
      else if (key == "username")
        username = value;
      else if (key == "note")
        note = value;
    }
  }

  if (action == "login" && !username.empty()) {
    Session &session =
        _client->server_config->clientSession_.createSession(username, note);
    setStatus(200);
    _headers = "Set-Cookie: " +
               _client->server_config->clientSession_.generateCookie(
                   session.getSessionId()) +
               "\r\n";
    _headers += "Content-Type: text/plain\r\n";
    _body = "status:success\nusername:" + username + "\nnote:" + note;
  } else if (action == "check_session") {
    std::string sessionId =
        parseCookies(_client->Srequest.headers["Cookie"], "sessionId");
    if (!sessionId.empty() &&
        _client->server_config->clientSession_.hasSession(sessionId)) {
      Session &session =
          _client->server_config->clientSession_.getSession(sessionId);
      setStatus(200);
      _body = "status:success\nusername:" + session.getUsername() +
              "\nnote:" + session.getNote();
    } else {
      if (error_page("401") == 0) {
        _body = "status:error\nmessage:No active session";
      }
    }
  } else if (action == "logout") {
    std::string sessionId =
        parseCookies(_client->Srequest.headers["Cookie"], "sessionId");
    if (!sessionId.empty()) {
      _client->server_config->clientSession_.removeSession(sessionId);
    }

    setStatus(200);
    _body = "status:success";
  } else {
    if (error_page("400") == 0) {
      _body = "status:error\nmessage:Invalid request";
    }
  }
}

bool Response::handleUploadRequest() {
  std::map<std::string, std::string>::iterator it_ =
      _client->Srequest.headers.find("Content-Type");

  if (it_ == _client->Srequest.headers.end() || it_->second != "text/plain") {
    delete_file(_client->Srequest.filename);
    if (error_page("415") == 0) {
      _body = "<html><body> < h1>415 Unsupported Media Type</h1 > < "
              "p>Unsupported media type.</p></body></html>";
      return true;
    }
  }

  std::map<std::string, std::string>::iterator it =
      _client->Srequest.headers.find("X-File-Name");
  std::string fileName;

  if (it != _client->Srequest.headers.end())
    fileName = it->second;
  else
    fileName = "uploaded_file";

  if (_client->route->upload_dir.empty()) {
    if (error_page("403") == 0) {
      _body = "<html><body> < h1>403 Forbidden</h1 > < p>Upload directory not "
              "configured.</p></body></html>";
    }
    return true;
  }

  std::string uploadDirPath = "." + _client->route->upload_dir;
  struct stat st;

  if (stat(uploadDirPath.c_str(), &st) != 0) {
    if (mkdir(uploadDirPath.c_str(), 0755) != 0) {
      if (error_page("500") == 0) {
        _body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Failed "
                "to create upload directory.</p></body></html>";
      }
      return true;
    }
  }

  std::string uploadPath = uploadDirPath + "/" + fileName;
  if (access(uploadPath.c_str(), F_OK) == 0) {
    if (error_page("409") == 0) {
      _body = "<html><body> < h1>409 Conflict</h1 > < p>File already "
              "exists.</p></body></html>";
    }
    return true;
  }

  if (rename(_client->Srequest.filename.c_str(), uploadPath.c_str()) != 0) {
    perror("rename failed");
    if (error_page("500") == 0) {
      _body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Failed to "
              "save uploaded file.</p></body></html>";
    }
  } else {
    setStatus(201);
    _body = "<html><body> < h1>201 Created</h1 > < p>File uploaded "
            "successfully.</p></body></html>";
  }
  return true;
}

bool Response::handlePostRequest() {
  size_t dotPos = _filePath.find_last_of(".");

  if (dotPos != std::string::npos) {
    std::string extension = _filePath.substr(dotPos);
    if (_client->route->cgi_extension.find(extension) !=
        _client->route->cgi_extension.end()) {
      return (handleCGIRequest());
    } else if (_client->Srequest.path == "/pages/login.html") {
      handleLoginRequest();
    } else if (_client->Srequest.path == "/submit-upload") {
      handleUploadRequest();
    } else {
      delete_file(_client->Srequest.filename);
      if (error_page("403") == 0) {
        _body = "<html><body> < h1>403 Forbidden</h1><p></p></body></html>";
      }
    }
  }

  return true;
}

void Response::handleDeleteRequest() {
  struct stat fileStat;

  if (stat(_filePath.c_str(), &fileStat) != 0) {
    if (error_page("404") == 0) {
      _body = "<html><body> < h1>404 Not Found</h1 > < p>The resource does not "
              "exist.</p></body></html>";
    }
    return;
  }

  if (S_ISDIR(fileStat.st_mode)) {
    if (error_page("403") == 0) {
      _body = "<html><body> < h1>403 Forbidden</h1 > < p>Cannot delete a "
              "directory.</p></body></html>";
    }
    return;
  }

  if (unlink(_filePath.c_str()) != 0) {
    if (error_page("500") == 0) {
      _body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Failed to "
              "delete the resource.</p></body></html>";
    }
    return;
  }

  setStatus(200);
  _body = "<html><body> < h1>200 OK</h1 > < p>Resource deleted "
          "successfully.</p></body></html>";
}

void Response::response_handler() {
  switch (_handlerState) {
  case HSTATE_ERROR_CHECK:

    if (_client->Srequest.error_status != 0) {
      if (error_page(to_string(_client->Srequest.error_status)) == 0) {
        _body = "<html><body><h1>" + _status +
                "</h1 > < p>An error occurred processing your "
                "request.</p></body></html>";
      }

      _handlerState = HSTATE_SET_HEADERS;
    } else {
      _handlerState = HSTATE_REDIRECT_CHECK;
    }

    break;

  case HSTATE_REDIRECT_CHECK:
    if (!_client->route->redirect.empty()) {
      setStatus(301);
      _headers = "Location: " + _client->route->redirect + "\r\n";
      _handlerState = HSTATE_SET_HEADERS;
    } else {
      _handlerState = HSTATE_RESOLVE_PATH;
    }

    break;

  case HSTATE_RESOLVE_PATH:
    resolveFilePath();
    _handlerState = HSTATE_HANDLE_METHOD;
    break;

  case HSTATE_HANDLE_METHOD:
    if (_client->Srequest.method == "GET") {
      if (!handleGetRequest())
        return;
    } else if (_client->Srequest.method == "POST") {
      if (!handlePostRequest())
        return;
    } else if (_client->Srequest.method == "DELETE") {
      handleDeleteRequest();
    } else {
      if (error_page("501") == 0) {
        _body = "<html><body> < h1>501 Not Implemented</h1 > < p>The requested "
                "method is not supported.</p></body></html>";
      }
    }

    _handlerState = HSTATE_SET_HEADERS;
    break;

  case HSTATE_SET_HEADERS:
    setHeaders();
    _headerBuffer = "HTTP/1.1 " + _status + "\r\n" + _headers;
    _headerSent = 0;
    _handlerState = HSTATE_SEND_HEADERS;
    break;

  case HSTATE_SEND_HEADERS: {
    if (sendResponseChunk(true)) {
      _bytesSent = 0;
      _handlerState = HSTATE_SEND_BODY;
    }

    break;
  }

  case HSTATE_SEND_BODY: {
    if (sendResponseChunk(false)) {
      _handlerState = HSTATE_COMPLETE;
    }

    break;
  }

  case HSTATE_COMPLETE:
    _client->set_response_status(Complete);
    std::cout << Logger::trace_http(
        "TRACE", _client->client_ip, _client->client_port,
        _client->Srequest.method, _client->Srequest.path,
        _client->Srequest.version, _client->res._status,
        to_string(current_time_in_ms() - _client->time_start_));
    break;

  case HSTATE_ERROR:
    break;
  }
}

bool Response::sendResponseChunk(bool sendHeader) {
  if (sendHeader) {
    if (_headerSent < _headerBuffer.size()) {
      ssize_t bytesSent =
          send(_client->get_socket_fd(), _headerBuffer.c_str() + _headerSent,
               _headerBuffer.size() - _headerSent, 0);
      if (bytesSent < 0) {
        std::cerr << "Error sending response headers to client FD "
                  << _client->get_socket_fd() << std::endl;
        return true;
      }

      _headerSent += bytesSent;
      _client->time_client_ = time(NULL);
      if (_headerSent < _headerBuffer.size())
        return false;
    }

    return true;
  }

  if (_body.size() > 0) {
    if (_bytesSent < _bytesToSend) {
      ssize_t bytesSent =
          send(_client->get_socket_fd(), _body.c_str() + _bytesSent,
               _bytesToSend - _bytesSent, 0);

      if (bytesSent < 0) {
        std::cerr << "Error sending response body to client FD "
                  << _client->get_socket_fd() << std::endl;
        return true;
      }

      _bytesSent += bytesSent;
      _client->time_client_ = time(NULL);
    }

    return (_bytesSent >= _bytesToSend);
  }

  if (_file_path_fd > 0) {
    switch (_sendState) {
    case SEND_IDLE:

    case SEND_READING:
      _bufferLen = read(_file_path_fd, _buffer, MAX_SEND);
      _bufferSent = 0;
      if (_bufferLen > 0) {
        _sendState = SEND_SENDING;
      } else {
        close_fd(_file_path_fd);
        _sendState = SEND_DONE;
        return true;
      }

    case SEND_SENDING:
      if (_bufferSent < _bufferLen) {
        ssize_t bytesSent =
            send(_client->get_socket_fd(), _buffer + _bufferSent,
                 _bufferLen - _bufferSent, 0);
        if (bytesSent < 0) {
          std::cerr << "Error sending file chunk to client FD "
                    << _client->get_socket_fd() << std::endl;
          close_fd(_file_path_fd);
          _sendState = SEND_DONE;
          return true;
        }

        _bufferSent += bytesSent;
        _client->time_client_ = time(NULL);
      }

      if (_bufferSent >= _bufferLen) {
        _sendState = SEND_READING;
      }
      return false;

    case SEND_DONE:
      return true;
    }
  }
  return false;
}
