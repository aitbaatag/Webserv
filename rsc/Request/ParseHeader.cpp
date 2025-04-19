#include "../../Includes/Http_Req_Res/Request.hpp"
#include <iostream>
#include <string>

bool HttpRequest::isStrucutredField(const std::string &field) {
  return structuredFields.find(field) != structuredFields.end();
}
std::string trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r\f\v");
  if (start == std::string::npos)
    return ""; // All spaces

  size_t end = str.find_last_not_of(" \t\n\r\f\v");
  return str.substr(start, end - start + 1);
}
bool ValidMediaType(const std::string &media_type) {

  printf("media_type: %s\n", media_type.c_str());
  if (media_type == "text/plain")
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
    std::cout << "Unsupported Media Type" << std::endl;
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
void HttpRequest::SetStartState(HttpClient &client) {
  if (client.Srequest.field_name == "Content-Length") {
    client.SMrequest.stateStructuredField = STATE_CONTENT_LENGTH;
  } else if (client.Srequest.field_name == "Date") {
    client.SMrequest.stateStructuredField = STATE_DATE;
  } else if (client.Srequest.field_name == "Content-Type") {
    client.SMrequest.stateStructuredField = STATE_CONTENT_TYPE;
  } else if (client.Srequest.field_name == "Cache-Control") {
    client.SMrequest.stateStructuredField = STATE_CACHE_CONTROL;
  } else if (client.Srequest.field_name == "Authorization") {
    client.SMrequest.stateStructuredField = STATE_AUTHORIZATION;
  } else if (client.Srequest.field_name == "Set-Cookie") {
    client.SMrequest.stateStructuredField = STATE_SET_COOKIE;
  } else {
    client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
  }
}
bool HttpRequest::ParseStructuredField(HttpClient &client) {
  char *reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < client.bytes_received) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateStructuredField) {
    case STATE_START:
      SetStartState(client);
      continue;
    case STATE_CONTENT_LENGTH:
      if (c == '\r') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
        client.Srequest.body_length = std::stoull(client.Srequest.field_body);
      } else if (c >= '0' && c <= '9') {
        client.Srequest.field_body += c;
      } else {
        return false;
      }
      break;
    case STATE_DATE:
      if (c == '\r') {
        // TODO : parse date
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
      } else {
        client.Srequest.field_body += c;
      }
      break;
    case STATE_CONTENT_TYPE:
      if (c == '\r') {
        if (!ParseContent_Type(client)) {
          return false;
        }
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
      } else {
        client.Srequest.field_body += c;
      }
      break;
    case STATE_CACHE_CONTROL:
      // TODO : parse cache control
      if (c == '\r') {
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
      } else {
        client.Srequest.field_body += c;
      }
      break;
    case STATE_AUTHORIZATION:
      // TODO : parse authorization
      if (c == '\r') {
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
      } else {
        client.Srequest.field_body += c;
      }
      break;
    case STATE_SET_COOKIE:
      // TODO : parse set cookie
      if (c == '\r') {
        client.SMrequest.stateStructuredField = STATE_STRUCTURED_FIELD_END;
      } else {
        client.Srequest.field_body += c;
      }
      break;
    case STATE_STRUCTURED_FIELD_END:
      if (c == '\n') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_START;
        client.update_pos(pos);
        if (pos >= client.bytes_received) {
          client.update_pos(0);
        }
        return true;
      } else {
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
  return true;
}

// parsing headeR
bool HttpRequest::parseHeaders(HttpClient &client) {
  char *reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (reqBuff[pos] != '\0') {
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
    case STATE_STRUCUTRE_FIELD:
      if (!ParseStructuredField(client)) {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      pos = client.get_pos();
      continue;
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
        if (isStrucutredField(client.Srequest.field_name)) {
          client.SMrequest.stateHeaders = STATE_STRUCUTRE_FIELD;
        } else
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
        std::cout << "Headers finished, body starts at position: " << pos
                  << std::endl;
        return true;
      } else
        client.SMrequest.stateHeaders = STATE_ERROR;
      break;
    case STATE_ERROR:
      std::cout << "Error" << std::endl;
      client.Srequest.field_name.clear();
      client.Srequest.field_body.clear();
      client.Srequest.error_status = 400; // Bad Request
      client.set_request_status(Failed);
      std::cout << "Bad Request" << std::endl;
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
