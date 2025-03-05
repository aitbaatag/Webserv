#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <iostream>
StateMachine::StateMachine() {
  stateRequestLine = STATE_METHOD;
  state = STATE_REQUEST_LINE;
  stateHeaders = STATE_HEADER_NAME;
  stateStructuredField = STATE_START;
  stateMultipart = START;
  statechunk = STATE_CHUNK_SIZE;
}
