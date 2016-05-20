#ifndef LOBBYSERVER_H
#define LOBBYSERVER_H

#include "action_type.h"
#include "action.h"

class Lobbyserver {
  private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;
    server m_server;
    con_list m_connections;
    std::queue<Action> m_actions;
    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
  public:
    Lobbyserver();
    void run(uint16_t port);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void process_messages();
};

#endif
