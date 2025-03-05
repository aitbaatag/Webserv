#include "../../Includes/Http_Req_Res/Request.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>

void printRequest(const HTTPRequest &req) {
  std::cout << "\n=== Request Details ===" << std::endl;
  std::cout << "Method: " << req.method << std::endl;
  std::cout << "URI: " << req.uri << std::endl;
  std::cout << "Version: " << req.httpVersion << std::endl;

  std::cout << "\n--- Headers ---" << std::endl;
  std::map<std::string, std::string>::const_iterator it;
  for (it = req.headers.begin(); it != req.headers.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }

  if (!req.body.empty()) {
    std::cout << "\n--- Body ---" << std::endl;
    std::cout << req.body << std::endl;
  }
  std::cout << "==================\n" << std::endl;
}

int main() {
  HTTPRequest parser;

  // Test 1: Simple GET request
  std::cout << "Test 1: Simple GET request" << std::endl;
  std::string getRequest = "GET /index.html HTTP/1.1\r\n"
                           "Host: localhost:8080\r\n"
                           "User-Agent: Mozilla/5.0\r\n"
                           "Accept: text/html\r\n"
                           "\r\n";

  HTTPRequest req1 = parser.parseRequest(getRequest);
  printRequest(req1);

  // Test 2: POST request with body
  std::cout << "Test 2: POST request with body" << std::endl;
  std::string postRequest =
      "POST /submit HTTP/1.1\r\n"
      "Host: localhost:8080\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: 25\r\n"
      "\r\n"
      "username=john&password=123";

  HTTPRequest req2 = parser.parseRequest(postRequest);
  printRequest(req2);

  // Test 3: DELETE request
  std::cout << "Test 3: DELETE request" << std::endl;
  std::string deleteRequest = "DELETE /file.txt HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "\r\n";

  HTTPRequest req3 = parser.parseRequest(deleteRequest);
  printRequest(req3);

  // Test 4: Malformed request (missing HTTP version)
  std::cout << "Test 4: Malformed request" << std::endl;
  try {
    std::string malformedRequest = "GET /index.html\r\n"
                                   "Host: localhost:8080\r\n"
                                   "\r\n";

    HTTPRequest req4 = parser.parseRequest(malformedRequest);
    printRequest(req4);
  } catch (const std::exception &e) {
    std::cout << "Error caught (as expected): " << e.what() << std::endl;
  }

  // Test 5: Request with multiple headers
  std::cout << "Test 5: Request with multiple headers" << std::endl;
  std::string complexRequest = "GET /api/data HTTP/1.1\r\n"
                               "Host: localhost:8080\r\n"
                               "User-Agent: Mozilla/5.0\r\n"
                               "Accept: text/html,application/xhtml+xml\r\n"
                               "Accept-Language: en-US,en;q=0.9\r\n"
                               "Accept-Encoding: gzip, deflate\r\n"
                               "Connection: keep-alive\r\n"
                               "Cookie: sessionId=abc123\r\n"
                               "\r\n";

  HTTPRequest req5 = parser.parseRequest(complexRequest);
  printRequest(req5);

  // Test 6: POST request with chunked encoding
  std::cout << "Test 6: POST request with chunked encoding" << std::endl;
  std::string chunkedRequest = "POST /upload HTTP/1.1\r\n"
                               "Host: localhost:8080\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "7\r\n"
                               "Mozilla\r\n"
                               "9\r\n"
                               "Developer\r\n"
                               "0\r\n"
                               "\r\n";

  HTTPRequest req6 = parser.parseRequest(chunkedRequest);
  printRequest(req6);

  // Test 7: Request with empty body but Content-Length: 0
  std::cout << "Test 7: Request with Content-Length: 0" << std::endl;
  std::string emptyBodyRequest = "POST /empty HTTP/1.1\r\n"
                                 "Host: localhost:8080\r\n"
                                 "Content-Length: 0\r\n"
                                 "\r\n";

  HTTPRequest req7 = parser.parseRequest(emptyBodyRequest);
  printRequest(req7);

  return 0;
}
