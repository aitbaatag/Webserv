#include "../../Includes/Http_Req_Res/Request.hpp"
#include <iostream>
#include <string>

std::string trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r\f\v");
  if (start == std::string::npos)
    return ""; // All spaces

  size_t end = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(start, end - start + 1);
}
bool ValidMediaType(const std::string &media_type) {
  if (media_type == "text/plain" ||
      media_type == "application/x-www-form-urlencoded" ||
      media_type == "multipart/form-data" || media_type == "application/json" ||
      media_type == "application/xml")
    return true;
  return false;
}
bool HttpRequest::ParseContent_Type(HttpClient &client) {
  std::string field_body = client.Srequest.field_body;
  std::map<std::string, std::string> content_type_params;
  std::string param;
  std::string media_type;
  std::string charset;
  std::string boundary;
  size_t semicolonPos = field_body.find(';');
  media_type = field_body.substr(0, semicolonPos);
  if (!ValidMediaType(media_type)) {
    client.Srequest.error_status = 415; // Unsupported Media Type
    client.set_request_status(Failed);
    return false;
  }
  field_body = field_body.substr(semicolonPos + 1);
  field_body = trim(field_body);
  std::istringstream field_body_stream(field_body);
  while (std::getline(field_body_stream, param, ';')) {
    size_t pos = param.find("=");
    std::string key = param.substr(0, pos);
    std::string value = param.substr(pos + 1);
    if (key == "charset")
      client.Srequest.charset = value;
    else
      content_type_params[key] = value;
  }
  return true;
}

// parsing headeR
bool HttpRequest::parseHeaders(HttpClient &client) {
  char *reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < client.bytes_received) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateHeaders) {
    case STATE_HEADER_NAME:
      if ((c > 32 && c < 127) && c != ':') {
        client.SMrequest.stateHeaders = STATE_HEADER_NAME;
        client.Srequest.field_name += c;
      } else if (c == ':') {
        client.SMrequest.stateHeaders = STATE_COLON;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_HEADER_VALUE:
      if (c == '\r') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
      } else if (c >= 32 && c < 127) {
        client.SMrequest.stateHeaders = STATE_HEADER_VALUE;
        client.Srequest.field_body += c;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_SPACE:
      if (c > 32 && c < 127) {
        client.SMrequest.stateHeaders = STATE_HEADER_VALUE;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      continue;
    case STATE_COLON:
      if (c == ' ') {
        client.SMrequest.stateHeaders = STATE_SPACE;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_HEADER_CRLF:
      if (c == '\n') {
        client.Srequest.headers[client.Srequest.field_name] =
            client.Srequest.field_body;
        if (client.Srequest.field_name == "Content-Length") {
          if (client.Srequest.field_body.empty()) {
            client.Srequest.error_status = 411; // Length Required
            client.set_request_status(Failed);
            return false;
          }
          if (client.Srequest.field_body.find_first_not_of("0123456789") !=
              std::string::npos) {
            client.Srequest.error_status = 400; // Bad Request
            client.set_request_status(Failed);
            return false;
          }
          client.Srequest.body_length =
              std::strtoull(client.Srequest.field_body.c_str(), NULL, 10);
        } else if (client.Srequest.field_name == "Content-Type") {
          if (!ParseContent_Type(client)) {
            client.SMrequest.stateHeaders = STATE_ERROR;
            return false;
          }
        }
        client.Srequest.field_name.clear();
        client.Srequest.field_body.clear();
        client.SMrequest.stateHeaders = STATE_HEADER_DELIMITER;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_HEADER_DELIMITER:
      if (c == '\r') {
        client.SMrequest.stateHeaders = STATE_HEADER_DELIMITER2;
        break;
      } else
        client.SMrequest.stateHeaders = STATE_HEADER_NAME;
      continue;
    case STATE_HEADER_DELIMITER2:
      if (c == '\n') {
        client.SMrequest.state = STATE_BODY;
        pos++;
        client.update_pos(pos);

        return true;
      } else
        client.SMrequest.stateHeaders = STATE_ERROR;
      break;
    case STATE_ERROR:
      client.Srequest.field_name.clear();
      client.Srequest.field_body.clear();
      client.Srequest.error_status = 400; // Bad Request
      client.set_request_status(Failed);
      return false;
    }
    pos++;
    client.update_pos(pos);
  }
  if (pos >= client.bytes_received) {
    client.update_pos(0);
  }
  return false;
}
