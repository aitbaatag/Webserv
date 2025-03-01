#include "../../Includes/Http_Req_Res/StateMachine.hpp"
#include <iostream>
StateMachine::StateMachine() {
  std::cout << "StateMachine Constructor" << std::endl;
  stateRequestLine = STATE_METHOD;
  state = STATE_REQUEST_LINE;
  stateHeaders = STATE_HEADER_NAME;
  stateStructuredField = STATE_START;
}
