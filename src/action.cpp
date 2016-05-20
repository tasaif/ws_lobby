#include "action.h"

Action::Action(ActionType t, connection_hdl h){
  type = t;
  hdl = h;
}

Action::Action(ActionType t, connection_hdl h, server::message_ptr m){
  type = t;
  hdl = h;
  msg = m;
}
