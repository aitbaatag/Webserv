#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../http_client/http_client.hpp"
#include "../libraries/Libraries.hpp"
#include <cstddef>

#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

class HttpRequest {
private:
  // parsing functions for request line
  static bool parseRequestLine(HttpClient &client);

  // parsing functions for headers
  static bool parseHeaders(HttpClient &client);

  // parsing functions for body
  static bool parseChunkedBody(HttpClient &client, const Route &route);
  static bool parseTextPlainBody(HttpClient &client, const Route &route);
  static bool directory_exists(const char *path);

  static void setFileName(const Route &route, HttpClient &client);
  // parsing functions for request line
  static bool validMethod(HttpClient &client, const std::string &method);
  static bool isAllowedURICharacter(char ch);
  static bool isValidURI(const std::string &uri);
  static bool validHttpVersion(const std::string &version);
  static char hexToChar(char hex1, char hex2);
  static void decodeRequestURI(HttpClient &client);
  static void parseURI(HttpClient &client);
  static bool ParseContent_Type(HttpClient &client);

  // parsing functions for body
  static BodyType
  determineBodyType(const std::map<std::string, std::string> &headers);

public:
  HttpRequest();
  static void parseIncrementally(HttpClient &client);
};

unsigned long hexToULong(const std::string &hexStr);
#endif
