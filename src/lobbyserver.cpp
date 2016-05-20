#include "lobbyserver.h"

Lobbyserver::Lobbyserver() {
  m_server.init_asio();
  m_server.set_open_handler(bind(&Lobbyserver::on_open,this,::_1));
  m_server.set_close_handler(bind(&Lobbyserver::on_close,this,::_1));
  m_server.set_message_handler(bind(&Lobbyserver::on_message,this,::_1,::_2));
}

void Lobbyserver::run(uint16_t port) {
  m_server.listen(port);
  m_server.start_accept();
  try {
    m_server.run();
  } catch (const std::exception & e) {
    std::cout << e.what() << std::endl;
  }
}

void Lobbyserver::on_open(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(action(SUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::on_close(connection_hdl hdl) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(action(UNSUBSCRIBE,hdl));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::on_message(connection_hdl hdl, server::message_ptr msg) {
  {
    lock_guard<mutex> guard(m_action_lock);
    m_actions.push(action(MESSAGE,hdl,msg));
  }
  m_action_cond.notify_one();
}

void Lobbyserver::process_messages() {
  while(1) {
    unique_lock<mutex> lock(m_action_lock);
    while(m_actions.empty()) {
      m_action_cond.wait(lock);
    }
    action a = m_actions.front();
    m_actions.pop();
    lock.unlock();
    if (a.type == SUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      m_connections.insert(a.hdl);
    } else if (a.type == UNSUBSCRIBE) {
      lock_guard<mutex> guard(m_connection_lock);
      m_connections.erase(a.hdl);
    } else if (a.type == MESSAGE) {
      lock_guard<mutex> guard(m_connection_lock);
      con_list::iterator it;
      for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(*it,a.msg);
      }
    } else {
    // undefined.
    }
  }
}
