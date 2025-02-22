#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <string>
#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define GET "GET"
#define POST "POST"
#define DELETE "DELETE"
enum StateRequestLine {
  STATE_METHOD,
  FIRST_SPACE,
  SECOND_SPACE,
  STATE_URI,
  STATE_VERSION
};
enum ParseState {
  STATE_REQUEST_LINE,
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
  ParseState state;
  StateRequestLine stateRequestLine;
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
  void validMethod(std::string &method);
  void validURI(std::string &uri);

public:
  HttpRequest(Request &Sreq);
  void parseIncrementally(std::string &reqBuff);
  bool isComplete();
};
#endif
