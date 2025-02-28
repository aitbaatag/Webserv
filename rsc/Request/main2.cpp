#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include "../../Includes/http_client/http_client.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main() {
  // HttpClient client(0);
  HttpRequest req;

  // Test 1
  // client.request_buffer_ = "G";
  // req.parseIncrementally(client);
  // client.request_buffer_ += "ET /inde";
  // req.parseIncrementally(client);
  // client.request_status_ = InProgress;
  // client.request_buffer_ += "x.html HTTP/1.1";
  // req.parseIncrementally(client);
  // client.request_buffer_ += "\r\n\nHost: localhost:8081\r\n";
  // req.parseIncrementally(client);
  // req.printRequestLine(client);

  // Test 2
  // with query and fragment
  HttpClient client2(0);
  client2.request_buffer_ = "GET /index.html HTTP/1.1\r\n";
  client2.request_buffer_ += "Host: localhost:8080\r\n";
  client2.request_buffer_ += "Content-Type: text/html; charset=UTF-8\r\n";
  client2.request_buffer_ += "Content-Length: 10\r\n";
  client2.request_buffer_ += "Date: Sun, 06 Nov 1994 08:49:37 GMT\r\n";
  client2.request_buffer_ += "Cache-Control: no-cache\r\n";
  client2.request_buffer_ +=
      "Authorization: Basic QW xhZGRpbjpvcGVuIHNlc2FtZQ==\r\n";
  client2.request_buffer_ +=
      "Set-Cookie: UserID=JohnDoe; Max-Age=3600; Version=1\r\n";
  client2.request_buffer_ += "Connection: keep-alive\r\n";
  client2.request_buffer_ += "\r\n";
  req.parseIncrementally(client2);
  req.printRequestLine(client2);

  // print map
  // for (auto const &[key, value] : client2.Srequest.headers) {
  //   std::cout << "{" << key << ": " << value << "}\n";
  // }
}
