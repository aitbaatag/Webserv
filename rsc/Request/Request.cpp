#include "../../Includes/Http_Req_Res/Request.hpp"
#include <cstddef>

HttpRequest::HttpRequest(std::string &request) : request(request) {}

void HttpRequest::parseRequestLine(const std::string &requestLine,
                                   HTTPRequest &httprequest) {
  httprequest.method = getMethod(requestLine);
  httprequest.uri = getUri(requestLine);
  httprequest.httpVersion = getHttpVersion(requestLine);
}

// private methods to parse the request requestLine
// example: GET /path/to/resource HTTP/1.1
// method: GET
std::string HttpRequest::getMethod(const std::string &requestLine) {
  std::string method;
  size_t start = 0;
  size_t end = requestLine.find(' ');
  method = requestLine.substr(start, end - start);
  return method;
}

// uri: /path/to/resource
