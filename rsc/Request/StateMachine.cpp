
#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <iostream>
StateMachine::StateMachine() {
  stateRequestLine = STATE_METHOD;
  state = STATE_REQUEST_LINE;
  stateHeaders = STATE_HEADER_NAME;
  stateTextPlain = createFile;
  stateChunk = STATE_FILE_CREATE;
  bodyType = START_;
}
