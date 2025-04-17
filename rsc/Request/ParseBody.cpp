#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <cstdarg>
#include <iostream>

bool HttpRequest::parseChunkedBody(HttpClient &client) { return true; }
bool HttpRequest::parseTextPlainBody(HttpClient &client) {
  std::string FileName = client.Srequest.headers["X-File-Name"];
  char *dataBuff = client.get_request_buffer() + client.get_pos();

  if (FileName.empty()) {
    client.Srequest.error_status = 400; // Bad Request
    client.set_request_status(Failed);
    std::cerr << "Bad Request" << std::endl;
    return true;
  }
  if (client.Srequest.body_length == 0) {
    std::cerr << "Empty body not allowed" << std::endl;
    client.Srequest.error_status = 411; // Length Required
    return true;
  }
  switch (client.SMrequest.stateTextPlain) {
  case createFile: {
    std::cout << "Create File" << std::endl;
    client.Srequest.fileStream.open(FileName, std::ios::out | std::ios::binary |
                                                  std::ios::trunc);
    if (!client.Srequest.fileStream.is_open()) {
      std::cerr << "Error opening file" << std::endl;
      client.Srequest.error_status = 500;
      client.set_request_status(Failed);
      return true;
    }
    client.SMrequest.stateTextPlain = writeFile;
  }
  case writeFile: {
      std::cout << "Write File" << std::endl;
      // std::cout << "fd client" << client.socket_fd_  << std::endl;
    if (client.Srequest.body_write < client.Srequest.body_length) {

      size_t pos = client.get_pos();
      size_t body_length = client.Srequest.body_length;
      size_t body_read = client.Srequest.body_write;
      size_t to_write = body_length - body_read;
      if (to_write > client.bytes_received - pos) {
        to_write = client.bytes_received - pos;
      }

      if (to_write > 0) {
        std::cout << client.Srequest.body_write << std::endl;
        client.Srequest.fileStream.write(dataBuff, to_write);
        if (!client.Srequest.fileStream ) {
          std::cerr << "Failed to write to file: " << FileName << std::endl;
          client.Srequest.error_status = 500;
          client.set_request_status(Failed);
          client.Srequest.fileStream.close();
          return true;
        }
        client.Srequest.body_write += to_write;
        client.update_pos(0);
      }
    }

    if (client.Srequest.body_write >= client.Srequest.body_length) {
      client.SMrequest.stateTextPlain = checkFile;
    } else
      return false; // Not fully processed
  }
  case checkFile: {
    std::cout << "Check File: Completed " << client.Srequest.body_write << "/"
              << client.Srequest.body_length << " bytes" << std::endl;

    client.Srequest.fileStream.close();
    return true; // Fully processed
  }
  }
  return false;
}
