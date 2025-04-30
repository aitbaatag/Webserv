#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/utlis/utils.hpp"
bool HttpRequest::directory_exists(const char *path) {
  DIR *dir = opendir(path);
  if (dir) {
    closedir(dir);
    return true; // Directory exists
  } else {
    return false; // Directory does not exist
  }
}

BodyType HttpRequest::determineBodyType(
    const std::map<std::string, std::string> &headers) {
  if (headers.find("Transfer-Encoding") != headers.end() &&
      headers.at("Transfer-Encoding") == "chunked") {
    return CHUNKED;
  } else if (headers.find("Content-Length") != headers.end()) {
    return TEXT_PLAIN;
  } else {
    return NO_BODY;
  }
}

unsigned long hexToULong(const std::string &hexStr) {
  unsigned long value = 0;
  for (size_t i = 0; i < hexStr.length(); i++) {
    char c = hexStr[i];
    value *= 16;
    if (c >= '0' && c <= '9') {
      value += c - '0';
    } else if (c >= 'A' && c <= 'F') {
      value += 10 + (c - 'A');
    } else if (c >= 'a' && c <= 'f') {
      value += 10 + (c - 'a');
    } else {
      throw std::invalid_argument("Invalid hexadecimal character");
    }
  }

  return value;
}

bool HttpRequest::validMethod(HttpClient &Clinet, const std::string &method) {
  if (method != GET && method != POST && method != DELETE) {
    return false;
  }

  return true;
}

void HttpRequest::parseURI(HttpClient &client) {
  std::string uri = client.Srequest.uri;
  size_t queryPos = uri.find("?");
  size_t fragmentPos = uri.find("#");

  if (fragmentPos != std::string::npos) {
    client.Srequest.fragment = uri.substr(fragmentPos + 1);
    uri = uri.substr(0, fragmentPos); // Remove fragment from URI
  }

  if (queryPos != std::string::npos) {
    client.Srequest.path = uri.substr(0, queryPos);
    client.Srequest.query = uri.substr(queryPos + 1);
  } else {
    client.Srequest.path = uri;
  }
}

char HttpRequest::hexToChar(char hex1, char hex2) {
  char c = 0;
  if (hex1 >= '0' && hex1 <= '9') {
    c = hex1 - '0';
  } else if (hex1 >= 'A' && hex1 <= 'F') {
    c = hex1 - 'A' + 10;
  } else if (hex1 >= 'a' && hex1 <= 'f') {
    c = hex1 - 'a' + 10;
  }

  c *= 16;
  if (hex2 >= '0' && hex2 <= '9') {
    c += hex2 - '0';
  } else if (hex2 >= 'A' && hex2 <= 'F') {
    c += hex2 - 'A' + 10;
  } else if (hex2 >= 'a' && hex2 <= 'f') {
    c += hex2 - 'a' + 10;
  }

  return c;
}

void HttpRequest::decodeRequestURI(HttpClient &client) {
  std::string uri = client.Srequest.uri;
  std::string decodedURI = "";
  for (size_t i = 0; i < uri.length(); ++i) {
    if (uri[i] == '%') {
      decodedURI += hexToChar(uri[i + 1], uri[i + 2]);
      i += 2;
    } else {
      decodedURI += uri[i];
    }
  }

  client.Srequest.uri = decodedURI;
}

// Unreserved characters: A-Z, a-z, 0-9, -, _, ., ~
// Reserved characters: :, /, ?, #, [, ], @, !, $, &, ', (,), *, +, ,, ;, =
// Percent-encoded characters (e.g., %20 for space) are Allowuered
bool HttpRequest::isAllowedURICharacter(char ch) {
  if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
      (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.' ||
      ch == '~') {
    return true;
  }

  if (ch == ':' || ch == '/' || ch == '?' || ch == '#' || ch == '[' ||
      ch == ']' || ch == '@' || ch == '!' || ch == '$' || ch == '&' ||
      ch == '\'' || ch == '(' || ch == ')' || ch == '*' || ch == '+' ||
      ch == ',' || ch == ';' || ch == '=') {
    return true;
  }

  if (ch == '%') {
    return true;
  }

  return false;
}

bool HttpRequest::isValidURI(const std::string &uri) {
  for (size_t i = 0; i < uri.length(); ++i) {
    if (uri[i] == '%') {
      if (i + 2 >= uri.length() || !isxdigit(uri[i + 1]) ||
          !isxdigit(uri[i + 2])) {
        return false;
      }
    } else if (!isAllowedURICharacter(uri[i])) {
      return false;
    }
  }

  return true;
}

bool HttpRequest::validHttpVersion(const std::string &version) {
  if (version != "HTTP/1.1") {
    return false;
  }

  return true;
}
