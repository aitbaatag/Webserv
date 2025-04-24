#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/Response.hpp"
#include <ios>
#include <iostream>
HttpRequest::HttpRequest() {}
bool HttpRequest::parseRequestLine(HttpClient &client) {
  char *reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < client.bytes_received) {
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
        if (pos >= client.bytes_received) {
          client.update_pos(0);
          return false;
        }
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
  if (pos >= client.bytes_received) {
    client.update_pos(0);
  }
  return false;
}

void HttpRequest::parseIncrementally(HttpClient &client,
                                     const std::vector<ServerConfig> &servers) {
  while (true) {
    switch (client.SMrequest.state) {
    case STATE_REQUEST_LINE:
      if (!parseRequestLine(client)) {
        return;
      }
      break;
    case STATE_HEADERS:
      if (!parseHeaders(client)) {
        return;
      }
    case STATE_BODY: {
      const ServerConfig &server =
          Response::findMatchingServer(servers, client.Srequest);
      const Route &route =
          Response::findMatchingRoute(server, client.Srequest.path);
      for (int i = 0; i < route.accepted_methods.size(); i++) {
        if (client.Srequest.method == route.accepted_methods[i]) {
          break;
        } else if (i == route.accepted_methods.size() - 1) {
          client.Srequest.error_status = 405;
          client.set_request_status(Failed);
          std::cout << "Method Not Allowed" << std::endl;
          return;
        }
      }
      switch (client.SMrequest.bodyType) {
      case START_: {
        std::cout << "Start" << std::endl;
        client.SMrequest.bodyType = determineBodyType(client.Srequest.headers);
        break;
      }
      case NO_BODY: {
        client.SMrequest.state = STATE_COMPLETE;
        break;
      }
      case CHUNKED: {
        std::cout << "Chunked" << std::endl;
        if (parseChunkedBody(client, route)) {
          client.SMrequest.state = STATE_COMPLETE;
        }
        return;
      }
      case TEXT_PLAIN: {
        // std::cout << "Text Plain" << std::endl;
        if (parseTextPlainBody(client, route)) {
          client.SMrequest.state = STATE_COMPLETE;
        }
        return;
      }
      }
      break;
    }
    case STATE_COMPLETE:
      client.set_request_status(Complete);
      return;
    }
  }
}

// void HttpRequest::printRequestLine(HttpClient &client) {
//   for (auto const &[key, value] : client.Srequest.headers) {
//     std::cout << "{" << key << ": " << value << "}\n";
//   }
// }
