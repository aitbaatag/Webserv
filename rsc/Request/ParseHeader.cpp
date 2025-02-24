#include "../../Includes/Http_Req_Res/Request.hpp"
#include <string>

bool HttpRequest::isStrucutredField(const std::string &field) {
  return structuredFields.find(field) != structuredFields.end();
}

bool ValidMediaType(const std::string &media_type) {
  if (media_type == "text/html" || media_type == "text/plain" ||
      media_type == "application/json" || media_type == "application/xml" ||
      media_type == "application/x-www-form-urlencoded" ||
      media_type == "multipart/form-data")
    return true;
  return false;
}
bool HttpRequest::ParseContent_Type(HttpClient &client) {
  std::string field_body;
  field_body = client.Srequest.headers["Content-Type"];
  std::map<std::string, std::string> content_type_params;
  std::string param;
  std::string media_type;
  std::string charset;
  std::string boundary;
  media_type = field_body.substr(0, field_body.find(";"));
  if (!ValidMediaType(media_type)) {
    client.Srequest.error_status = 415; // Unsupported Media Type
    client.set_request_status(Failed);
    std::cout << "Unsupported Media Type" << std::endl;
    return false;
  }
  field_body = field_body.substr(field_body.find(";") + 1);
  std::istringstream field_body_stream(field_body);
  while (std::getline(field_body_stream, param, ';')) {
    size_t pos = param.find("=");
    std::string key = param.substr(0, pos);
    std::string value = param.substr(pos + 1);
    if (key == "charset")
      client.Srequest.charset = value;
    else if (key == "boundary") {
      client.Srequest.boundary = value;
    } else
      content_type_params[key] = value;
  }
  client.Srequest.Content_Type_ =
      std::make_pair(media_type, content_type_params);
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
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos() + 1;
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateStructuredField) {
    case STATE_START:
      SetStartState(client);
    case STATE_CONTENT_LENGTH:
      if (c == '\r') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_START;
        client.Srequest.body_length = std::stoi(client.Srequest.field_body);
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
        client.SMrequest.stateStructuredField = STATE_START;
      } else {
        client.Srequest.field_body += c;
      }
    case STATE_CONTENT_TYPE:
      if (c == '\r') {
        if (!ParseContent_Type(client))
          return false;
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_START;
      } else {
        client.Srequest.field_body += c;
      }
    case STATE_CACHE_CONTROL:
      // TODO : parse cache control
    case STATE_AUTHORIZATION:
      // TODO : parse authorization
    case STATE_SET_COOKIE:
      // TODO : parse set cookie
    case STATE_STRUCTURED_FIELD_END:
      if (c == '\n') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
        client.SMrequest.stateStructuredField = STATE_START;
        return true;
      } else {
        return false;
      }
      break;
    }
    pos++;
    client.update_pos(pos);
  }
  return true;
}

// parsing headeR
bool HttpRequest::parseHeaders(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();
  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.SMrequest.stateHeaders) {
    case STATE_HEADER_NAME:
      std::cout << "STATE_HEADER_NAME" << std::endl;
      if ((c < 32 && c < 127) && c != ':') {
        client.SMrequest.stateHeaders = STATE_HEADER_NAME;
        client.Srequest.field_name += c;
      } else if (c == ':') {
        client.SMrequest.stateHeaders = STATE_COLON;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_STRUCUTRE_FIELD:
      ParseStructuredField(client);
      break;
    case STATE_HEADER_VALUE:
      if (c == '\r') {
        client.SMrequest.stateHeaders = STATE_HEADER_CRLF;
      } else if ((c < 32 && c < 127)) {
        client.SMrequest.stateHeaders = STATE_HEADER_VALUE;
        client.Srequest.field_body += c;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_COLON:
      if (c == ' ') {
        if (isStrucutredField(client.Srequest.field_name)) {
          client.SMrequest.stateHeaders = STATE_STRUCUTRE_FIELD;
        } else
          client.SMrequest.stateHeaders = STATE_HEADER_VALUE;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
    case STATE_HEADER_CRLF:
      if (c == '\n') {
        client.Srequest.headers[client.Srequest.field_name] =
            client.Srequest.field_body;
        client.Srequest.field_name.clear();
        client.Srequest.field_body.clear();
        client.SMrequest.stateHeaders = STATE_HEADER_NAME;
      } else {
        client.SMrequest.stateHeaders = STATE_ERROR;
      }
      break;
    case STATE_ERROR:
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

  return true;
}
