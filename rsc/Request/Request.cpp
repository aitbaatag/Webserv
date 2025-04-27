#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/Response.hpp"
#include "../../Includes/server/server_socket.hpp"

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

void HttpRequest::parseIncrementally(HttpClient &client) {
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
      ServerConfig &server = *Response::findMatchingServer(client.server->getServerConfig(), client.Srequest);
      const Route &route =   *Response::findMatchingRoute(server, client.Srequest.path);
      for (int i = 0; i < route.accepted_methods.size(); i++) {
        if (client.Srequest.method == route.accepted_methods[i]) {
          break;
        } else if (i == route.accepted_methods.size() - 1) {
          client.Srequest.error_status = 405;
          client.set_request_status(Failed);
          return;
        }
      }

      switch (client.SMrequest.bodyType) {
      case START_: {
        client.SMrequest.bodyType = determineBodyType(client.Srequest.headers);
        if (client.Srequest.body_length > server.max_body_size) {
          client.Srequest.error_status = 413; // Payload Too Large
          client.set_request_status(Failed);
          std::cerr << "Request body too large: " << client.Srequest.body_length
                    << " bytes (max: " << server.max_body_size << " bytes)"
                    << std::endl;
          return;
        }

        break;
      }

      case NO_BODY: {
        client.SMrequest.state = STATE_COMPLETE;
        break;
      }

      case CHUNKED: {
        if (parseChunkedBody(client, route)) {
          client.SMrequest.state = STATE_COMPLETE;
          continue;
        }

        return;
      }

      case TEXT_PLAIN: {
        if (parseTextPlainBody(client, route)) {
          client.SMrequest.state = STATE_COMPLETE;
          continue;
        }

        return;
      }
      }

      break;
    }

    case STATE_COMPLETE:
      if (client.Srequest.fileStream.is_open()) {
        client.Srequest.fileStream.flush();
        client.Srequest.fileStream.seekg(0);
      }

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
