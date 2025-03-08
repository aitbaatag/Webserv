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

  client2.request_buffer_ = "GET /index.html?query=1#fragment HTTP/1.1\r\n"
                            "Host: localhost:8081\r\n"
                            "Connection: keep-alive\r\n";
  client2.request_buffer_ +=
      "accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/"
      "webp,image/apng,*/*;q=0.8\r\n"
      "accept-encoding: gzip, deflate, br, zstd\r\n"
      "accept-language: en-US,en;q=0.5\r\n"
      "cache-control: max-age=0\r\n"
      "connection: keep-alive\r\n"
      "content-length: 24978572\r\n"
      "content-type: multipart/form-data; "
      "boundary=----WebKitFormBoundaryewgKoIys71od9Vsi\r\n"
      "host: localhost:8081\r\n"
      "origin: http://localhost:8081\r\n"
      "referer: http://localhost:8081/home\r\n"
      "sec-ch-ua: \"Not(A:Brand\";v=\"99\", \"Brave\";v=\"133\", "
      "\"Chromium\";v=\"133\"\r\n"
      "sec-ch-ua-mobile: ?0\r\n"
      "sec-ch-ua-platform: \"Linux\"\r\n"
      "sec-fetch-dest: document\r\n"
      "sec-fetch-mode: navigate\r\n"
      "sec-fetch-site: same-origin\r\n"
      "sec-fetch-user: ?1\r\n"
      "sec-gpc: 1\r\n"
      "upgrade-insecure-requests: 1\r\n"
      "user-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/133.0.0.0 Safari/537.36"
      "\r\n";

  // bodu chunked
  client2.request_buffer_ += "Transfer-Encoding: chunked\r\n";
  client2.request_buffer_ += "\r\n";
  // chunked with hex size
  // client2.request_buffer_ +=
  //     "7\r\n{\"name\"\r\n8\r\n:\"John "
  //     "D\r\n6\r\noe\",\"a\r\n7\r\nge\":30,\r\n9\r\n\"email\":\"\r\n11\r\njohn@"
  //     "example.com\"\r\n1\r\n}\r\n0\r\n\r\n";
  client2.request_buffer_ += "5\r\n";
  client2.request_buffer_ += "Hello\r\n";
  client2.request_buffer_ += "5\r\n";
  client2.request_buffer_ += "World\r\n";
  client2.request_buffer_ += "1\r\n";
  client2.request_buffer_ += "!\r\n";
  client2.request_buffer_ += "0\r\n";
  client2.request_buffer_ += "\r\n";
  req.parseIncrementally(client2);
  req.printRequestLine(client2);

  // print map
  for (auto const &[key, value] : client2.Srequest.headers) {
    std::cout << "{" << key << ": " << value << "}\n";
  }
}
