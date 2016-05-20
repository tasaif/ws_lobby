#ifndef ACTION_H
#define ACTION_H

#include "global.h"
#include "action_type.h"
struct Action {
  Action(ActionType t, connection_hdl h);
  Action(ActionType t, connection_hdl h, server::message_ptr m);
  ActionType type;
  websocketpp::connection_hdl hdl;
  server::message_ptr msg;
};

#endif
