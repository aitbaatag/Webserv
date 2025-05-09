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
  static bool parseChunkedBody(HttpClient &client);
  static bool parseTextPlainBody(HttpClient &client);
  static bool directory_exists(const char *path);

  // parsing functions for request line
  static bool validMethod(const std::string &partial);
  
  static bool isAllowedURICharacter(char ch);
  static bool validHttpVersion(const std::string &partial);
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
