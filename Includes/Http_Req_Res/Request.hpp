#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../http_client/http_client.hpp"
#include <string>
// for trim function
#include <algorithm>
#include <cctype>
#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"

class HttpRequest {
private:
  // parsing functions for request line
  bool parseRequestLine(HttpClient &client);

  // parsing functions for headers
  bool parseHeaders(HttpClient &client);

  // parsing functions for body
  bool parseChunkedBody(HttpClient &client);
  bool StorMultipartBody(HttpClient &client);
  void parseChunkSize(HttpClient &client);
  void parseChunkData(HttpClient &client);
  void parseChunkEnd(HttpClient &client);
  bool parseBody(HttpClient &client);
  std::string getWriteData(std::string buffer, std::string boundary,
                           HttpClient &client);
  // parsing functions for request line
  bool validMethod(HttpClient &client, const std::string &method);
  bool isAllowedURICharacter(char ch);
  bool isValidURI(const std::string &uri);
  bool validHttpVersion(const std::string &version);
  char hexToChar(char hex1, char hex2);
  void decodeRequestURI(HttpClient &client);
  void parseURI(HttpClient &client);
  bool parseCRLF(HttpClient &client);
  // parsing functions for headers
  bool isStrucutredField(const std::string &field);
  bool ParseStructuredField(HttpClient &client);
  void SetStartState(HttpClient &client);
  bool ParseContent_Type(HttpClient &client);

  // parsing functions for body
  BodyType determineBodyType(const std::map<std::string, std::string> &headers);

public:
  HttpRequest();
  void parseIncrementally(HttpClient &client);
  void printRequestLine(HttpClient &client);
};

unsigned long hexToULong(const std::string &hexStr);
#endif
