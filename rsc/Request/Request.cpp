#include "../../Includes/Http_Req_Res/Request.hpp"
#include <ios>
#include <iostream>
HttpRequest::HttpRequest() {}
bool HttpRequest::parseRequestLine(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateRequestLine) {
    case STATE_METHOD:
      std::cout << "Method" << std::endl;
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
      std::cout << "URI" << std::endl;
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
      std::cout << "Version" << std::endl;
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
      std::cout << "CRLF" << std::endl;
      if (c == '\n') {
        client.SMrequest.state = STATE_HEADERS;
        if (reqBuff[pos] == '\n') {
          pos++; // Skip the '\n'
        }
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
  return false;
}

void HttpRequest::parseIncrementally(HttpClient &client) {
  while (true) {
    switch (client.SMrequest.state) {
    case STATE_REQUEST_LINE:
      std::cout << "Request Line" << std::endl;
      if (!parseRequestLine(client)) {
        client.update_pos(0);
        return;
      }
      break;
    case STATE_HEADERS:
      std::cout << "Headers" << std::endl;
      if (!parseHeaders(client)) {
        client.update_pos(0);
        return;
      }
    case STATE_BODY: {
      switch (client.SMrequest.bodyType) {
      case START_: {
        std::cout << "Start" << std::endl;
        client.SMrequest.bodyType = determineBodyType(client.Srequest.headers);
        if (client.SMrequest.bodyType == MULTIPART ||
            client.SMrequest.bodyType == CHUNKED) {
          if (!client.Srequest.tmpFileStream.is_open()) {
            client.Srequest.tmpFileStream.open(
                ".temp_file_" + std::to_string(client.get_socket_fd()) + ".txt",
                std::ios::app | std::ios::binary);
            if (!client.Srequest.tmpFileStream.is_open()) {
              std::cerr << "Failed to open temp file" << std::endl;
              client.Srequest.error_status = 500; // Internal Server Error
              client.set_request_status(Failed);
              return;
            }
          }
        }
        break;
      }
      case NO_BODY: {
        std::cout << "No Body" << std::endl;
        client.SMrequest.state = STATE_COMPLETE;
        break;
      }
      case MULTIPART: {
        if (!StorMultipartBody(client))
          client.SMrequest.state = STATE_COMPLETE;
        else {
          client.SMrequest.state = STATE_BODY;
          client.update_pos(0);
          return;
        }
        break;
      }
      case CHUNKED: {
        std::cout << "Chunked" << std::endl;
        if (!parseChunkedBody(client))
          return;
        break;
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

void HttpRequest::printRequestLine(HttpClient &client) {
  std::cout << "Method: " << client.Srequest.method << std::endl;
  std::cout << "URI: " << client.Srequest.uri << std::endl;
  std::cout << "Path: " << client.Srequest.path << std::endl;
  std::cout << "Version: " << client.Srequest.version << std::endl;
  std::cout << "Query: " << client.Srequest.query << std::endl;
  std::cout << "Fragment: " << client.Srequest.fragment << std::endl;
  // for (auto const &[key, value] : client.Srequest.headers) {
  //   std::cout << "{" << key << ": " << value << "}\n";
  // }
}
