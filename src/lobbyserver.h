#ifndef LOBBYSERVER_H
#define LOBBYSERVER_H

#include "action_type.h"
#include "action.h"
#include "connection_data.h"

class Lobbyserver {
  private:
    void init();
    typedef std::map<connection_hdl, ConnectionData,std::owner_less<connection_hdl> > con_list;
    server m_server;
    con_list m_connections;
    std::queue<Action> m_actions;
    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
    string validation_url;

  public:
    Lobbyserver();
    Lobbyserver(string _validation_url);
    void run(uint16_t port);
    void on_open(connection_hdl hdl);
    void close_con(connection_hdl, string);
    void send_con(connection_hdl, string);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void process_messages();
    void process_request(connection_hdl hdl, Json::Value msg);
    void broadcast(connection_hdl src, string);
    void broadcast(connection_hdl src, string, int dst_lobby);
};

#endif
