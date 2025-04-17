#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include "../libraries/Libraries.hpp"

enum StateHeaders {
  STATE_HEADER_NAME,
  STATE_HEADER_VALUE,
  STATE_COLON,
  STATE_HEADER_CRLF,
  STATE_STRUCUTRE_FIELD,
  STATE_HEADER_DELIMITER,
  STATE_HEADER_DELIMITER2,
  STATE_SPACE,
  STATE_ERROR
};
enum StateChunk {
  STATE_CHUNK_SIZE,
  STATE_CHUNK_DATA,
  STATE_CHUNK_END,
  STATE_CHUNK_LF, // Line Feed like '\n' after '\r'
  STATE_CHUNK_CRLF
};
enum Text_plain { createFile, writeFile, checkFile };
enum StateStructuredField {
  STATE_START,
  STATE_CONTENT_LENGTH,
  STATE_DATE,
  STATE_CONTENT_TYPE,
  STATE_CACHE_CONTROL,
  STATE_AUTHORIZATION,
  STATE_SET_COOKIE,
  STATE_STRUCTURED_FIELD_END
};
enum ParseState {
  STATE_REQUEST_LINE,
  STATE_HEADERS,
  STATE_BODY,
  STATE_COMPLETE
};
enum BodyType { START_, CHUNKED, TEXT_PLAIN, NO_BODY };
enum StateRequestLine { STATE_METHOD, STATE_URI, STATE_VERSION, STATE_CRLF };
const std::set<std::string> structuredFields = {
    "Content-Length", "Date",          "Content-Type",
    "Cache-Control",  "Authorization", "Set-Cookie"};
class StateMachine {
public:
  StateRequestLine stateRequestLine;
  ParseState state;
  StateHeaders stateHeaders;
  StateStructuredField stateStructuredField;
  StateChunk stateChunk;
  Text_plain stateTextPlain;
  BodyType bodyType;
  StateMachine();
};
#endif
