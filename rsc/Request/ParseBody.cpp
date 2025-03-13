#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <cstddef>
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

bool HttpRequest::StorMultipartBody(HttpClient &client) {
  std::string reqBuff = client.get_request_buffer();

  std::cout << "--------------------------Storing Multipart Body--------------"
            << std::endl;
  if (client.Srequest.body_read < client.Srequest.body_length) {
    size_t pos = client.get_pos();
    size_t body_length = client.Srequest.body_length;
    size_t body_read = client.Srequest.body_read;
    size_t to_read = body_length - body_read;
    if (to_read > reqBuff.length() - pos) {
      to_read = reqBuff.length() - pos;
    }
    write(client.Srequest.temp_file_fd, reqBuff.c_str() + pos, to_read);
    client.Srequest.body_read += to_read;
    pos += to_read;
    client.update_pos(pos);
  }

  if (client.Srequest.body_read == client.Srequest.body_length) {
    // return (parseBody(client));
    return false;
  }
  return false;
}

bool HttpRequest::parseBody(HttpClient &client) {
  const size_t CHUNK_SIZE = 16384; // 16KB
  char buffer[CHUNK_SIZE];
  bool true_ = true;

  if (!client.Srequest.tmpFileStream.is_open()) {
    client.Srequest.tmpFileStream.open(".temp_file",
                                       std::ios::in | std::ios::binary);
    if (!client.Srequest.tmpFileStream) {
      std::cerr << "Failed to open temporary file" << std::endl;
      client.Srequest.error_status = 500; // Internal Server Error
      client.set_request_status(Failed);
      return false;
    }
  }

  client.Srequest.tmpFileStream.read(buffer, CHUNK_SIZE);
  std::streamsize bytesRead = client.Srequest.tmpFileStream.gcount();
  buffer[bytesRead] = '\0';
  std::string str_buffer(buffer);
  while (true_) {
    switch (client.SMrequest.stateMultipart) {
    case OPEN_BOUNDARY: {
      std::string boundary = "--" + client.Srequest.boundary;
      size_t boundaryPos = str_buffer.find(boundary);
      if (boundaryPos == std::string::npos) {
        std::cerr << "Boundary not found" << std::endl;
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        client.Srequest.tmpFileStream.close();
        return false;
      }
      str_buffer = str_buffer.substr(boundaryPos + boundary.length());
      client.SMrequest.stateMultipart = HEADERS;
      break;
    }
    case CLOSE_BOUNDARY: {
    }
    case CREATEFILE: {
    }
    case HEADERS: {
      size_t headerEnd = str_buffer.find(CRLFCRLF);
      std::string line = str_buffer.substr(0, headerEnd);
      size_t posfilename = line.find("filename");
      if (posfilename != std::string::npos) {
        size_t poStart = posfilename + 10;
        size_t posEnd = line.find("\"", poStart);
        client.Srequest.filename = line.substr(poStart, posEnd - poStart);
        str_buffer = str_buffer.substr(headerEnd + 4);
      }
      break;
    }

    case DATA: {
      std::string writeData =
          getWriteData(str_buffer, client.Srequest.boundary, client);
      client.Srequest.fileStream.write(writeData.c_str(), writeData.length());
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
      return false;
      break;
    }
    }
  }
  return true;
}

std::string HttpRequest::getWriteData(std::string buffer, std::string boundary,
                                      HttpClient &client) {
  std::string endBoundary = "--" + boundary + "--";
  size_t boundaryPos = buffer.find(endBoundary);
  if (boundaryPos == std::string::npos) {
    return buffer;
  }
  client.SMrequest.stateMultipart = END_BOUNDARY;
  return buffer.substr(0, boundaryPos);
}
