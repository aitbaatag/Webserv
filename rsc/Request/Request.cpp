#include "../../Includes/Http_Req_Res/Request.hpp"
#include <cstddef>
#include <string>

HttpRequest::HttpRequest(Request &Sreq) : Srequest(Sreq) {
  Srequest.state = STATE_REQUEST_LINE;
  Srequest.complete = false;
  Srequest.error_status = 0;
}
bool HttpRequest::isComplete() { return Srequest.complete; }
void HttpRequest::validMethod(char c) {}
void HttpRequest::parseRequestLine(std::string &reqBuff) {
  size_t pos = 0;
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (Srequest.state) {
    case STATE_METHOD:
      if (c == 'G') {
        Srequest.method += c;
        Srequest.state = STATE_G_METHOD;
      } else if (c == 'P') {
        Srequest.method += c;
        Srequest.state = STATE_P_METHOD;
      } else if (c == 'D') {
        Srequest.method += c;
        Srequest.state = STATE_D_METHOD;
      } else
        Srequest.error_status = 405;
    }
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
