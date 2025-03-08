#ifndef REQUEST_HPP
#define REQUEST_HPP

<<<<<<< HEAD
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

enum class BodyType { CHUNKED, MULTIPART, NO_BODY };
class HttpRequest {
private:
  // parsing functions for request line
  bool parseRequestLine(HttpClient &client);

  // parsing functions for headers
  bool parseHeaders(HttpClient &client);

  // parsing functions for body
  bool parseChunkedBody(HttpClient &client);
  bool MultipartBody(HttpClient &client);
  void parseChunkSize(HttpClient &client);
  void parseChunkData(HttpClient &client);
  void parseChunkEnd(HttpClient &client);

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
=======
#include <map>
#include <string>
enum ParseState {
  STATE_REQUEST_LINE,
  STATE_METHOD,
  STATE_G_METHOD,
  STATE_P_METHOD,
  STATE_D_METHOD,
  STATE_URI,
  STATE_VERSION,
  STATE_HEADERS,
  STATE_BODY,
  STATE_CHUNK_SIZE,
  STATE_CHUNK_DATA,
  STATE_CHUNK_END,
  STATE_COMPLETE
};
struct Request {
  std::string method;
  std::string uri;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  ParseState state;
  size_t chunk_size;
  size_t body_length;
  bool complete;    // the request is complete
  int error_status; // the error status of the request
};

/*class HTTPRequestError {*/
/*public:*/
/*  std::map<int, const std::string> error_map = {*/
/*      {405, "Method Not Allowed"},*/
/*      {404, "Not Found"},*/
/*      {505, "HTTP Version Not Supported"},*/
/*      {501, "Not Implemented"},*/
/*      {502, "Bad Gateway"},*/
/*      {503, "Service Unavailable"},*/
/*      {504, "Gateway Timeout"},*/
/*      {505, "HTTP Version Not Supported"},*/
/*      {506, "Variant Also Negotiates"},*/
/*      {507, "Insufficient Storage"},*/
/*      {508, "Loop Detected"},*/
/*      {510, "Not Extended"},*/
/*      {511, "Network Authentication Required"},*/
/*      {599, "Network Connect Timeout Error"},*/
/*  };*/
/*};*/

class HttpRequest {
private:
  Request &Srequest;
  void parseRequestLine(std::string &reqBuff);
  void parseHeaders(std::string &reqBuff);
  void parseBody(std::string &reqBuff);
  void parseChunkSize(std::string &reqBuff);
  void parseChunkData(std::string &reqBuff);
  void parseChunkEnd(std::string &reqBuff);

public:
  HttpRequest(Request &Sreq);
  void parseIncrementally(std::string &reqBuff);
  bool isComplete();
};
>>>>>>> main
#endif
