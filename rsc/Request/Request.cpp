#include "../../Includes/Http_Req_Res/Request.hpp"
#include <iostream>
HttpRequest::HttpRequest() {}
bool HttpRequest::parseRequestLine(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateRequestLine) {
    case STATE_METHOD:
      if (c == ' ') {
        if (!validMethod(client, client.Srequest.method)) {
          client.Srequest.error_status = 405; // Method Not Allowed
          client.set_request_status(Failed);
          std::cout << "Method Not Allowed" << std::endl;
          return false;
        }
        client.SMrequest.stateRequestLine = STATE_URI;
      } else {
        client.Srequest.method += c;
      }
      break;
    case STATE_URI:
      if (c == ' ') {
        if (client.Srequest.uri[0] != '/' || !isValidURI(client.Srequest.uri)) {
          client.Srequest.error_status = 404; // Not Found
          client.set_request_status(Failed);
          std::cout << "Not Found" << std::endl;
          return false;
        } else {
          decodeRequestURI(client);
          parseURI(client);
          client.SMrequest.stateRequestLine = STATE_VERSION;
        }
      } else {
        client.Srequest.uri += c;
      }
      break;
    case STATE_VERSION:
      if (c == '\r') {
        if (!validHttpVersion(client.Srequest.version)) {
          client.Srequest.error_status = 505; // HTTP Version Not Supported
          client.set_request_status(Failed);
          std::cout << "HTTP Version Not Supported" << std::endl;
          return false;
        } else {
          client.SMrequest.stateRequestLine = STATE_CRLF;
        }
      } else {
        client.Srequest.version += c;
      }
      break;
    case STATE_CRLF:
      if (c == '\n') {
        client.SMrequest.state = STATE_HEADERS;
        pos++; // Skip the '\n'
        client.update_pos(pos);
        return true; // Successfully parsed the request line
      } else {
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        std::cout << "Bad Request (Invalid CRLF)" << std::endl;
        return false;
      }
      break;
    }
    pos++;
    client.update_pos(pos);
  }
  return true;
}

void HttpRequest::parseIncrementally(HttpClient &client) {
  size_t pos = client.get_pos();
  std::string reqBuff = client.get_request_buffer();
  while (client.get_pos() < reqBuff.length()) {
    switch (client.SMrequest.state) {
    case STATE_REQUEST_LINE:
      if (!parseRequestLine(client))
        return;
      break;
    case STATE_HEADERS:
      if (!parseHeaders(client))
        return;
      break;
    case STATE_BODY:
      // parseBody(client);
      break;
    case STATE_CHUNK_SIZE:
      // parseChunkSize(client);
      break;
    case STATE_CHUNK_DATA:
      // parseChunkData(client);
      break;
    case STATE_CHUNK_END:
      // parseChunkEnd(client);
      break;
    default:
      break;
    }
  }
  client.set_request_status(Complete);
}

void HttpRequest::printRequestLine(HttpClient &client) {
  // std::cout << "Method: " << client.Srequest.method << std::endl;
  // std::cout << "URI: " << client.Srequest.uri << std::endl;
  // std::cout << "Path: " << client.Srequest.path << std::endl;
  // std::cout << "Version: " << client.Srequest.version << std::endl;
  // std::cout << "Query: " << client.Srequest.query << std::endl;
  // std::cout << "Fragment: " << client.Srequest.fragment << std::endl;
  for (auto const &[key, value] : client.Srequest.headers) {
    std::cout << "{" << key << ": " << value << "}\n";
  }
}
