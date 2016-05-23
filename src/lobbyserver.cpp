#include "lobbyserver.h"

Lobbyserver::Lobbyserver() {
  m_server.clear_access_channels(websocketpp::log::alevel::all);
  m_server.init_asio();
  m_server.set_open_handler(bind(&Lobbyserver::on_open,this,::_1));
  m_server.set_close_handler(bind(&Lobbyserver::on_close,this,::_1));
  m_server.set_message_handler(bind(&Lobbyserver::on_message,this,::_1,::_2));
}

void Lobbyserver::run(uint16_t port) {
  m_server.listen(port);
  m_server.start_accept();
  try {
    cout << "Running server..." << endl;
    m_server.run();
  } catch (const std::exception & e) {
    std::cout << e.what() << std::endl;
  }
}

void Lobbyserver::on_open(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(Action(SUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::close_con(websocketpp::connection_hdl hdl, string msg){
  cout << "Terminating Connection" << endl;
  websocketpp::lib::error_code ec;
  m_server.close(hdl, websocketpp::close::status::normal, msg, ec);
  if (ec) {
    cout << ec.message() << endl;
  }
}

void Lobbyserver::on_close(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(Action(UNSUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::on_message(connection_hdl hdl, server::message_ptr msg) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(Action(MESSAGE,hdl,msg));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::process_messages() {
  Json::Reader reader;
  Json::Value msg_root;   // will contains the root value after parsing.
  while(1) {
    unique_lock<mutex> lock(m_action_lock);
    while(m_actions.empty()) {
      m_action_cond.wait(lock);
    }
    Action a = m_actions.front();
    m_actions.pop();
    lock.unlock();
    if (a.type == SUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      ConnectionData data;
      m_connections[a.hdl] = data;
      cout << "Client connected" << endl;
    } else if (a.type == UNSUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      close_con(a.hdl, "UNSUBSCRIBING");
      m_connections.erase(a.hdl);
      cout << "Client disconnected" << endl;
    } else if (a.type == MESSAGE) {
      lock_guard<mutex> guard(m_connection_lock);
      con_list::iterator it;
      for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(it->first,a.msg);
      }
      bool parsingSuccessful = reader.parse(a.msg->get_payload(), msg_root);
      if (parsingSuccessful){
        cout << "Parsed successful" << endl;
      } else {
        cout << "Error: Unrecognized request." << endl;
        on_close(a.hdl);
      }
      cout << "Message: " << a.msg->get_payload() << endl;
    } else {
    // undefined.
    }
  }
}
