#include "../../Includes/Http_Req_Res/Request.hpp"
#include <cstddef>
#include <string>

HttpRequest::HttpRequest(Request &Sreq) : Srequest(Sreq) {
  Srequest.state = STATE_REQUEST_LINE;
  Srequest.complete = false;
  Srequest.error_status = 0;
}
bool HttpRequest::isComplete() { return Srequest.complete; }
void HttpRequest::validMethod(std::string &method) {
  if (method != GET && method != POST && method != DELETE) {
    Srequest.error_status = 405;
    Srequest.complete = true;
    std::cout << "Method Not Allowed" << std::endl;
  }
}
void HttpRequest::validURI(std::string &uri) {
  if (uri[0] != '/') {
    Srequest.error_status = 404;
    Srequest.complete = true;
    std::cout << "Not Found" << std::endl;
  }
}
void HttpRequest::parseRequestLine(std::string &reqBuff) {
  size_t pos = 0;
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (Srequest.stateRequestLine) {
    case STATE_METHOD:
      if (c == ' ') {
        validMethod(Srequest.method);
        Srequest.stateRequestLine = FIRST_SPACE;
      } else {
        Srequest.method += c;
      }
      break;
    case FIRST_SPACE:
      if (c == ' ') {
        Srequest.stateRequestLine = STATE_URI;
      } else {
        Srequest.error_status = 400; // Bad Request
        Srequest.complete = true;
        std::cout << "Bad Request" << std::endl;
      }
      break;
    case STATE_URI:
      if (c == ' ') {
        validURI(Srequest.uri);
        Srequest.stateRequestLine = SECOND_SPACE;
      } else {
        Srequest.uri += c;
      }
      break;
    case SECOND_SPACE:
      if (c == ' ') {
        Srequest.stateRequestLine = STATE_VERSION;
      } else {
        Srequest.error_status = 400; // Bad Request
        Srequest.complete = true;
        std::cout << "Bad Request" << std::endl;
      }
      break;
    case STATE_VERSION:
      // TODO
    }
    pos++;
  }
}

void HttpRequest::parseIncrementally(std::string &reqBuff) {
  size_t pos = 0;
  while (pos < reqBuff.length()) {

    switch (Srequest.state) {
    case STATE_REQUEST_LINE:
      parseRequestLine(reqBuff);
      break;
    case STATE_HEADERS:
      parseHeaders(reqBuff);
      break;
    case STATE_BODY:
      parseBody(reqBuff);
      break;
    case STATE_CHUNK_SIZE:
      parseChunkSize(reqBuff);
      break;
    case STATE_CHUNK_DATA:
      parseChunkData(reqBuff);
      break;
    case STATE_CHUNK_END:
      parseChunkEnd(reqBuff);
      break;
    default:
      break;
    }
  }
}
