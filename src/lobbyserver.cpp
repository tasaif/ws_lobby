#include "lobbyserver.h"

void Lobbyserver::init(){
  m_server.clear_access_channels(websocketpp::log::alevel::all);
  m_server.init_asio();
  m_server.set_open_handler(bind(&Lobbyserver::on_open,this,::_1));
  m_server.set_close_handler(bind(&Lobbyserver::on_close,this,::_1));
  m_server.set_message_handler(bind(&Lobbyserver::on_message,this,::_1,::_2));
}

Lobbyserver::Lobbyserver() {
  init();
}

Lobbyserver::Lobbyserver(string _validation_url) {
  validation_url = _validation_url;
  init();
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

void Lobbyserver::close_con(connection_hdl hdl, string msg){
  cout << "Terminating Connection" << endl;
  websocketpp::lib::error_code ec;
  m_server.close(hdl, websocketpp::close::status::normal, msg, ec);
  if (ec) {
    cout << ec.message() << endl;
  }
}

void Lobbyserver::send_con(connection_hdl hdl, string msg){
  m_server.get_con_from_hdl(hdl)->send(msg);
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
      int cid = m_connections[a.hdl].client_id;
      int lid = m_connections[a.hdl].lobby_id;
      m_connections.erase(a.hdl);
      cout << "Client disconnected" << endl;
      if (lid != -1){
        notify_the_family(cid, lid);
      }
    } else if (a.type == MESSAGE) {
      lock_guard<mutex> guard(m_connection_lock);
      bool parsingSuccessful = reader.parse(a.msg->get_payload(), msg_root);
      if (parsingSuccessful){
        process_request(a.hdl, msg_root);
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

void Lobbyserver::process_request(connection_hdl hdl, Json::Value msg){
  if (msg["request"] == "connect"){
    Json::StreamWriterBuilder wbuilder;
    Json::Reader reader;
    Json::Value verification_msg;
    bool parsingSuccessful;
    unsigned lobby_id;
    int client_id = rand() % RAND_MAX;
    ostringstream url;
    ostringstream stringified_verification;
    curlpp::Easy request;
    lobby_id = msg["lobby"].asInt();
    cout << "Requested to join lobby: " << lobby_id << endl;
    url << validation_url << lobby_id;
    request.setOpt(new curlpp::options::Url(url.str()));
    stringified_verification << request;
    parsingSuccessful = reader.parse(stringified_verification.str(), verification_msg);
    if (parsingSuccessful){
      if (verification_msg["error"].asBool()){
        cout << "Error: lobby db responded with error" << endl;
        cout << verification_msg["text"] << endl;
        on_close(hdl);
      } else {
        cout << "Verified lobby request" << endl;
        m_connections[hdl].lobby_id = lobby_id;
        m_connections[hdl].client_id = client_id;
        verification_msg["id"] = Json::Value(client_id);
        verification_msg["from"] = Json::Value("server");
        verification_msg["type"] = Json::Value("response");
        verification_msg["message"] = Json::Value("verified");
        con_list::iterator it;
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
          int dst_id = m_connections[it->first].client_id;
          verification_msg["client_list"].append(Json::Value(dst_id));
        }
        std::string confirmation = Json::writeString(wbuilder, verification_msg);
        send_con(hdl, confirmation);
        //Notify clients of new connection
        verification_msg["message"] = Json::Value("new_client");
        string new_connection_message = Json::writeString(wbuilder, verification_msg);
        broadcast(hdl, new_connection_message, lobby_id);
      }
    } else {
      cout << "Error: lobby db responded with unparsible response" << endl;
      cout << stringified_verification << endl;
    }
  } else if (msg["request"] == "send"){
    Json::StreamWriterBuilder wbuilder;
    msg["from"] = Json::Value("neighbor");
    std::string to_one = Json::writeString(wbuilder, msg);
    send(hdl, to_one, m_connections[hdl].lobby_id, msg["id"].asInt());
  } else if (msg["request"] == "broadcast"){
    Json::StreamWriterBuilder wbuilder;
    msg["from"] = Json::Value("neighbor");
    msg["id"] = Json::Value(m_connections[hdl].client_id);
    std::string to_all = Json::writeString(wbuilder, msg);
    broadcast(hdl, to_all, m_connections[hdl].lobby_id);
  } else {
    cout << "Error: Unrecognized request." << endl;
    on_close(hdl);
  }
}

void Lobbyserver::broadcast(connection_hdl src, string msg, int dst_lobby){
  con_list::iterator it;
  int src_id = m_connections[src].client_id;
  for (it = m_connections.begin(); it != m_connections.end(); ++it) {
    if (m_connections[it->first].lobby_id == dst_lobby){
      int dst_id = m_connections[it->first].client_id;
      if (src_id != dst_id){
        send_con(it->first, msg);
      }
    }
  }
}

void Lobbyserver::broadcast(string msg, int dst_lobby){
  con_list::iterator it;
  for (it = m_connections.begin(); it != m_connections.end(); ++it) {
    if (m_connections[it->first].lobby_id == dst_lobby){
      send_con(it->first, msg);
    }
  }
}

void Lobbyserver::send(connection_hdl src, string msg, int dst_lobby, int dst_id){
  con_list::iterator it;
  for (it = m_connections.begin(); it != m_connections.end(); ++it) {
    if (m_connections[it->first].lobby_id == dst_lobby){
      int check_id = m_connections[it->first].client_id;
      int check_lobby = m_connections[it->first].lobby_id;
      if (check_id == dst_id && check_lobby == dst_lobby){
        send_con(it->first, msg);
        break;
      }
    }
  }
}

void Lobbyserver::notify_the_family(int the_departed, int dst_lobby){
  Json::StreamWriterBuilder wbuilder;
  Json::Value notification_msg;
  notification_msg["from"] = Json::Value("server");
  notification_msg["message"] = Json::Value("client_disconnected");
  notification_msg["id"] = Json::Value(the_departed);
  std::string to_all = Json::writeString(wbuilder, notification_msg);
  broadcast(to_all, dst_lobby);
}
