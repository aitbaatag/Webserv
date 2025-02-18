#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
struct HTTPRequest {
  std::string method;      // GET, POST, DELETE
  std::string uri;         // /path/to/resource
  std::string httpVersion; // HTTP/1.1
  std::map<std::string, std::string> headers;
  std::string body;
};

class HttpRequest {
private:
public:
  void parseRequestLine(const std::string &requestLine,
                        HTTPRequest &httprequest);
};
#endif
