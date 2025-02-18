#include "../../Includes/Http_Req_Res/Request.hpp"
#include <cstddef>
#include <iterator>
#include <sstream>

HttpRequest::HttpRequest(std::string &request) : request(request) {}
// parse http request line example: GET /path/to/resource HTTP/1.1
void HttpRequest::parseRequestLine(const std::string &requestLine,
                                   HTTPRequest &httprequest) {
  std::istringstream iss(requestLine);

  iss >> httprequest.method;
  iss >> httprequest.uri;
  iss >> httprequest.httpVersion;

  // TODO error handling for invalid request line
}

// parse http request headers
// example:
// GET /index.html HTTP/1.1
// Host: localhost:8080
// User-Agent: Mozilla/5.0
// Accept: text/html,application/xhtml+xml
// Accept-Language: en-US,en;q=0.9
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
// Cookie: sessionId=abc123
// Content-Length: 0

void HttpRequest::parseHeaders(const std::string &headers,
                               HTTPRequest &httprequest) {
  std::istringstream iss(headers);
  std::string line;
  // SKIP the first line
  iss >> line;
  while (std::getline(iss, line)) {
    if (line == "\r" || line.empty()) {
      break;
    }
    size_t colonPos = line.find(":");
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      std::string value = line.substr(colonPos + 1);
      size_t firstNonSpace = value.find_first_not_of(" ");
      if (firstNonSpace != std::string::npos) {
        value = value.substr(firstNonSpace);
      }
      if (value.back() == '\r') {
        value.pop_back(); // remove '\r' at the end of value
      }
      httprequest.headers[key] = value;
    }
  }
}

HTTPRequest HttpRequest::parseRequest(std::string &request) {
  HTTPRequest httprequest;
  std::istringstream iss(request);
  std::string requestLine;
  std::string headers;
  std::string body;

  std::getline(iss, requestLine);
  parseRequestLine(requestLine, httprequest);

  std::string line;
  while (std::getline(iss, line)) {
    if (line == "\r" || line.empty()) {
      break;
    }
    headers += line + "\n";
  }
  parseHeaders(headers, httprequest);

  std::getline(iss, body);
  httprequest.body = body;
  return httprequest;
}
