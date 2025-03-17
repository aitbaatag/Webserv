#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>

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

bool HttpRequest::StorMultipartBody(HttpClient &client) {
  char *reqBuff = client.get_request_buffer() + client.get_pos();
  if (client.Srequest.body_read < client.Srequest.body_length) {
    size_t pos = client.get_pos();
    size_t body_length = client.Srequest.body_length;
    size_t body_read = client.Srequest.body_read;
    size_t to_read = body_length - body_read;
    if (to_read > client.bytes_received - pos) {
      to_read = client.bytes_received - pos;
    }

    client.Srequest.tmpFileStream.write(reqBuff, to_read);
    if (!client.Srequest.tmpFileStream) {
      std::cerr << "Failed to write to temp file" << std::endl;
      client.Srequest.error_status = 500; // Internal Server Error
      client.set_request_status(Failed);
      return false;
    }

    client.Srequest.tmpFileStream.flush();
    pos += to_read;
    client.Srequest.body_read += to_read;
    client.update_pos(pos);
  }

  if (client.Srequest.body_read == client.Srequest.body_length) {
    return (parseBody(client));
    return false;
  }
  return true;
}

bool HttpRequest::parseBody(HttpClient &client) {
  const size_t CHUNK_SIZE = 16000; // 16KB
  char buffer[CHUNK_SIZE];
  bool true_ = true;

  if (!client.Srequest.tmpFileStream.is_open()) {
    std::cerr << "Failed to open temp file" << std::endl;
    client.Srequest.error_status = 500; // Internal Server Error
    client.set_request_status(Failed);
    return false;
  }
  client.Srequest.tmpFileStream.clear();
  client.Srequest.tmpFileStream.seekg(0, std::ios::beg);
  if (!client.Srequest.tmpFileStream) {
    std::cerr << "Failed to move to beginning of temp file" << std::endl;
    client.Srequest.error_status = 500; // Internal Server Error
    client.set_request_status(Failed);
    return false;
  }

  client.Srequest.tmpFileStream.read(buffer, CHUNK_SIZE - 1);
  size_t bytesRead = client.Srequest.tmpFileStream.gcount();
  if (!client.Srequest.tmpFileStream) {
    std::cerr << "Failed to read from temp file" << std::endl;
    client.Srequest.error_status = 500; // Internal Server Error
    client.set_request_status(Failed);
    return false;
  }
  while (true_) {
    switch (client.SMrequest.stateMultipart) {
    case OPEN_BOUNDARY: {
      std::cout << "Open Boundary" << std::endl;
      std::string boundary = "--" + client.Srequest.boundary;
      for (size_t i = 0; i < boundary.length(); i++) {
        std::cout << boundary << std::endl;
        std::cout << buffer[i] << std::endl;
        if (memcmp(buffer + i, boundary.c_str(), boundary.length()) == 0) {
          std::cout << "Boundary found" << std::endl;
          client.SMrequest.stateMultipart = HEADERS;
          memmove(buffer, buffer + i, bytesRead - i);
          break;
        }
      }
      break;
    }
    case CLOSE_BOUNDARY: {
    }
    case CREATEFILE: {
      client.Srequest.fileStream.open(client.Srequest.filename,
                                      std::ios::out | std::ios::binary |
                                          std::ios::trunc);
      if (!client.Srequest.fileStream) {
        std::cerr << "Failed to create file: " << client.Srequest.filename
                  << std::endl;
        client.Srequest.error_status = 500; // Internal Server Error
        client.set_request_status(Failed);
        client.Srequest.tmpFileStream.close();
        return false;
      }
      client.SMrequest.stateMultipart = DATA;
      break;
    }
    case HEADERS: {
      std::string headers = "";
      for (size_t i = 0; i < bytesRead; i++) {
        if (memcmp(buffer + i, CRLFCRLF, 4) == 0) {
          std::cout << "Header found" << std::endl;
          client.SMrequest.stateMultipart = CREATEFILE;
          headers = std::string(buffer, i + 4);
          memmove(buffer, buffer + i + 4, bytesRead - i - 4);
          break;
        }
      }
      size_t headerEnd = headers.find(CRLFCRLF);
      if (headerEnd == std::string::npos) {
        std::cerr << "Header not found" << std::endl;
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        client.Srequest.tmpFileStream.close();
        return false;
      }
      size_t posfilename = headers.find("filename");
      if (posfilename != std::string::npos) {
        size_t poStart = posfilename + 10;
        size_t posEnd = headers.find("\"", poStart);
        client.Srequest.filename = headers.substr(poStart, posEnd - poStart);
        client.SMrequest.stateMultipart = CREATEFILE;
      } else {
        std::cerr << "Filename not found" << std::endl;
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        client.Srequest.tmpFileStream.close();
        return false;
      }
      break;
    }

    case DATA: {
      bytesRead = getWriteData(buffer, bytesRead, client);
      client.Srequest.fileStream.write(buffer, bytesRead);
      if (!client.Srequest.fileStream) {
        std::cerr << "Failed to write to file: " << client.Srequest.filename
                  << std::endl;
        client.Srequest.error_status = 500; // Internal Server Error
        client.set_request_status(Failed);
        client.Srequest.tmpFileStream.close();
        client.Srequest.fileStream.close();
        return false;
      }
      true_ = false;
      break;
    }
    case END_BOUNDARY: {
      std::cout << "End Boundary" << std::endl;
      client.Srequest.tmpFileStream.close();
      client.Srequest.fileStream.close();
      std::remove(client.Srequest.tmpFilename.c_str());
      return false;
      break;
    }
    }
  }
  return true;
}
size_t HttpRequest::getWriteData(char *buffer, size_t buffer_size,
                                 HttpClient &client) {
  std::string endBoundary = "--" + client.Srequest.boundary + "--";

  // Search for end boundary in the buffer
  // for (size_t i = 0; i <= buffer_size - endBoundary.length(); i++) {
  //   if (memcmp(buffer + i, endBoundary.c_str(), endBoundary.length()) == 0) {
  //     // End boundary found at position i
  //     client.SMrequest.stateMultipart = END_BOUNDARY;
  //
  //     // Return the position of the boundary
  //     return i;
  //   }
  // }
  //
  // No end boundary found, return the entire buffer size
  return buffer_size;
}
