#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"

bool HttpRequest::parseChunkedBody(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();
  size_t pos = client.get_pos();

  while (pos < reqBuff.length()) {
    char c = reqBuff[pos];
    switch (client.SMrequest.statechunk) {
    case STATE_CHUNK_SIZE:
      if (c == '\r') {
        try {
          client.Srequest.chunkSize = hexToULong(client.Srequest.chunkSizeStr);
          client.Srequest.chunkSizeStr.clear();
          client.SMrequest.statechunk = STATE_CHUNK_LF;
        } catch (const std::exception &e) {
          client.Srequest.error_status = 400;
          client.set_request_status(Failed);
          std::cout << "Bad Request (Invalid chunk size)" << std::endl;
          return false;
        }
      } else if (c == '\n') {
        client.Srequest.error_status = 400;
        client.set_request_status(Failed);
        std::cout << "Bad Request (Invalid chunk size format)" << std::endl;
        return false;
      } else {
        client.Srequest.chunkSizeStr += c;
      }
      break;
    case STATE_CHUNK_DATA:
      if (client.Srequest.chunkSize == 0) {
        client.SMrequest.statechunk = STATE_CHUNK_END;
        break;
      }
      client.Srequest.chunkData =
          reqBuff.substr(pos, client.Srequest.chunkSize);
      pos += client.Srequest.chunkSize;
      client.Srequest.chunkSize = 0;
      client.SMrequest.statechunk = STATE_CHUNK_CRLF;
      break;

    case STATE_CHUNK_CRLF:
      if (c == '\n') {
        // store data in temp temp_file_fd
        write(client.Srequest.temp_file_fd, client.Srequest.chunkData.c_str(),
              client.Srequest.chunkData.length());
        client.Srequest.chunkData.clear();
        client.SMrequest.statechunk = STATE_CHUNK_SIZE;
      } else {
        client.Srequest.error_status = 400;
        client.set_request_status(Failed);
        std::cout << "Bad Request (Expected LF)" << std::endl;
        return false;
      }
      break;

    case STATE_CHUNK_LF:
      if (c == '\n') {
        if (client.Srequest.chunkSize == 0) {
          client.SMrequest.statechunk = STATE_CHUNK_END;
        } else {
          client.SMrequest.statechunk = STATE_CHUNK_DATA;
        }
      } else {
        client.Srequest.error_status = 400;
        client.set_request_status(Failed);
        std::cout << "Bad Request (Expected CR)" << std::endl;
        return false;
      }
      break;

    case STATE_CHUNK_END:
      client.set_request_status(Complete);
      return true;

    default:
      client.Srequest.error_status = 500;
      client.set_request_status(Failed);
      std::cout << "Internal Server Error (Invalid state)" << std::endl;
      return false;
    }
    pos++;
    client.update_pos(pos);
  }
  return true;
}

bool HttpRequest::MultipartBody(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();

  // strore the body in a temp temp_file_fd and check if the body is Complete
  std::cout << "--------------------- storing body in temp temp_file_fd "
            << client.Srequest.temp_file_fd << std::endl;
  if (client.Srequest.body_read < client.Srequest.body_length) {
    size_t pos = client.get_pos();
    size_t body_length = client.Srequest.body_length;
    size_t body_read = client.Srequest.body_read;
    size_t to_read = body_length - body_read;
    if (to_read > reqBuff.length() - pos) {
      to_read = reqBuff.length() - pos;
    }
    std::cout << reqBuff << std::endl;
    write(client.Srequest.temp_file_fd, reqBuff.c_str() + pos, to_read);
    client.Srequest.body_read += to_read;
    pos += to_read;
    client.update_pos(pos);
  }

  if (client.Srequest.body_read == client.Srequest.body_length) {
    client.set_request_status(Complete);
    return true;
  }
  return false;
}
