#include "../../Includes/Http_Req_Res/Request.hpp"

// parsing headeR
bool HttpRequest::parseHeaders(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.Srequest.state) {
    case:
      STATE_HEADER_NAME

    case:
      STATE_HEADER_VALUE

    case:
      STATE_COLON

    case:
      STATE_HEADER_CRLF

    case:
      STATE_ERROR
    }
    pos++;
    client.update_pos(pos);
  }

  return true;
}
