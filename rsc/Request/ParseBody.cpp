#include "../../Includes/Http_Req_Res/Request.hpp"
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include "../../Includes/utlis/utils.hpp"
#include <cstdarg>
#include <iostream>
#include <iterator>
#include <string>

bool HttpRequest::parseChunkedBody(HttpClient &client) {
  char *reqBuff = client.get_request_buffer() + client.get_pos();

  // check if upload directory exists if not put file in troot
  client.Srequest.filename = ".tmp_file_" + to_string(client.socket_fd_);

  size_t to_write = 0;
  size_t pos = client.get_pos();
  while (true) {
    switch (client.SMrequest.stateChunk) {
    case STATE_FILE_CREATE: {
      client.Srequest.fd_file = open(client.Srequest.filename.c_str(),
                                     O_RDWR | O_CREAT | O_TRUNC, 0666);
      if (client.Srequest.fd_file < 0) {
        client.Srequest.error_status = 500;
        client.set_request_status(Failed);
        return true;
      }
      client.SMrequest.stateChunk = STATE_CHUNK_SIZE;
      continue;
    }

    case STATE_CHUNK_SIZE: {
      while (pos < client.bytes_received) {
        char c = reqBuff[pos];

        if (c == '\r') {
          try {
            client.Srequest.chunk_size =
                std::strtoull(client.Srequest.chunk_size_str.c_str(), NULL, 16);
            client.SMrequest.stateChunk = STATE_CHUNK_LF;
            pos++;
            break;
          }

          catch (const std::exception &e) {
            client.Srequest.error_status = 400; // Bad Request
            client.set_request_status(Failed);
            return true;
          }
        } else if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F')) {
          client.Srequest.chunk_size_str += c;
          pos++;
        } else {
          client.Srequest.error_status = 400; // Bad Request
          client.set_request_status(Failed);
          return true;
        }
      }

      if (pos >= client.bytes_received) {
        return false; // Need more data
      }

      continue;
    }

    case STATE_CHUNK_LF: {
      if (pos >= client.bytes_received) {
        return false; // Need more data
      }

      char c = reqBuff[pos];
      if (c == '\n') {
        pos++;

        if (client.Srequest.chunk_size == 0) {
          client.SMrequest.stateChunk = STATE_CHUNK_END;
        } else {
          client.SMrequest.stateChunk = STATE_CHUNK_DATA;
          client.Srequest.chunk_bytes_read = 0;
        }
      } else {
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        return true;
      }

      continue;
    }

    case STATE_CHUNK_DATA: {
      // Calculate how much data we can process
      size_t remaining =
          client.Srequest.chunk_size - client.Srequest.chunk_bytes_read;
      size_t available = client.bytes_received - pos;
      to_write = (remaining < available) ? remaining : available;

      if (to_write > 0) {
        write(client.Srequest.fd_file, reqBuff + pos, to_write);
        if (client.Srequest.fd_file < 0) {
          client.Srequest.error_status = 500;
          client.set_request_status(Failed);
          close_fd(client.Srequest.fd_file);
          return true;
        }
        client.Srequest.chunk_bytes_read += to_write;
        client.Srequest.body_write += to_write;
        pos += to_write;
      }

      if (client.Srequest.chunk_bytes_read >= client.Srequest.chunk_size) {
        client.SMrequest.stateChunk = STATE_CHUNK_CRLF;
      } else {
        // Need more data
        return false;
      }

      continue;
    }

    case STATE_CHUNK_CRLF: {
      if (pos >= client.bytes_received) {
        return false; // Need more data
      }

      char c = reqBuff[pos];

      if (c == '\r') {
        pos++;
        if (pos >= client.bytes_received) {
          return false; // Need more data
        }

        c = reqBuff[pos];
        if (c == '\n') {
          pos++;
          // Reset for next chunk
          client.SMrequest.stateChunk = STATE_CHUNK_SIZE;
          client.Srequest.chunk_size_str.clear();
        } else {
          client.Srequest.error_status = 400; // Bad Request
          client.set_request_status(Failed);
          close_fd(client.Srequest.fd_file);
          return true;
        }
      } else if (c == '\n') {
        pos++;
        client.SMrequest.stateChunk = STATE_CHUNK_SIZE;
        client.Srequest.chunk_size_str.clear();
      } else {
        client.Srequest.error_status = 400; // Bad Request
        client.set_request_status(Failed);
        close_fd(client.Srequest.fd_file);
        return true;
      }

      continue;
    }

    case STATE_CHUNK_END: {
      if (pos >= client.bytes_received) {
        return false; // Need more data
      }

      char c = reqBuff[pos];

      if (c == '\r') {
        pos++;
        if (pos >= client.bytes_received) {
          return false; // Need more data
        }

        c = reqBuff[pos];
        if (c == '\n') {
          pos++;
          return true;
        }
      } else if (c == '\n') {
        pos++;
        return true;
      }

      client.Srequest.error_status = 400; // Bad Request
      client.set_request_status(Failed);
      if (client.Srequest.fd_file >= 0) {
        close_fd(client.Srequest.fd_file);
      }

      return true;
    }
    }
  }

  return true;
}

bool HttpRequest::parseTextPlainBody(HttpClient &client) {
  std::string FileName;
  client.Srequest.filename = ".tmp_file_" + to_string(client.socket_fd_);
  char *dataBuff = client.get_request_buffer() + client.get_pos();
  size_t to_write = 0;

  // check if content length header is present
  if (client.Srequest.body_length <= 0) {
    std::cerr << "Empty body not allowed" << std::endl;
    client.Srequest.error_status = 411; // Length Required
    client.set_request_status(Failed);
    return true;
  }

  while (true) {
    switch (client.SMrequest.stateTextPlain) {
    case createFile: {
      client.Srequest.fd_file = open(client.Srequest.filename.c_str(),
                                     O_RDWR | O_CREAT | O_TRUNC, 0666);
      if (client.Srequest.fd_file < 0) {
        std::cerr << "Error opening file" << std::endl;
        client.Srequest.error_status = 500;
        client.set_request_status(Failed);
        return true;
      }
      client.SMrequest.stateTextPlain = ValidData;
      continue;
    }

    case ValidData: {
      if (client.Srequest.body_write < client.Srequest.body_length) {
        size_t pos = client.get_pos();
        size_t body_length = client.Srequest.body_length;
        size_t body_read = client.Srequest.body_write;
        to_write = body_length - body_read;
        if (to_write > client.bytes_received - pos) {
          to_write = client.bytes_received - pos;
        }
      }
      client.SMrequest.stateTextPlain = writeFile;
      continue;
    }

    case writeFile: {
      if (to_write > 0) {
        size_t bytes_write = write(client.Srequest.fd_file, dataBuff, to_write);

        if (client.Srequest.fd_file < 0 || bytes_write < 0) {
          std::cerr << "Failed to write to file: " << client.Srequest.filename
                    << std::endl;
          client.Srequest.error_status = 500;
          client.set_request_status(Failed);
          close_fd(client.Srequest.fd_file);
          return true;
        }

        client.Srequest.body_write += to_write;
        client.update_pos(0);
      }

      if (client.Srequest.body_write >= client.Srequest.body_length) {
        client.SMrequest.stateTextPlain = checkFile;
        continue;
      }

      client.SMrequest.stateTextPlain = ValidData;
      return false; // Not fully processed
    }

    case checkFile: {
      return true; // Fully processed
    }
    }

    break;
  }

  return false;
}
