#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <string>
struct HTTPRequest {
  std::string method;      // GET, POST, DELETE
  std::string uri;         // /path/to/resource
  std::string httpVersion; // HTTP/1.1
  std::map<std::string, std::string> headers;
  std::string body;
};

class HttpRequest {
private:
  std::string &request;
  std::string getMethod(const std::string &requestLine);
  std::string getUri(const std::string &requestLine);
  std::string eetHttpVersion(const std::string &requestLine);
  std::map<std::string, std::string> getHeaders(const std::string &headers);
  std::string getBody(const std::string &body);

public:
  HttpRequest(std::string &request);
  void parseRequestLine(const std::string &requestLine,
                        HTTPRequest &httprequest);
  void parseHeaders(const std::string &headers, HTTPRequest &httprequest);
  void parseBody(const std::string &body, HTTPRequest &httprequest);
  void parseRequest(const std::string &request, HTTPRequest &httprequest);
  void printRequest(const HTTPRequest &httprequest);
  void printRequestLine(const HTTPRequest &httprequest);
  void printHeaders(const HTTPRequest &httprequest);
  void printBody(const HTTPRequest &httprequest);
};
#endif
