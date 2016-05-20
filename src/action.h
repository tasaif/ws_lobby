#ifndef ACTION_H
#define ACTION_H

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, connection_hdl h, server::message_ptr m)
      : type(t), hdl(h), msg(m) {}
    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};

#endif
