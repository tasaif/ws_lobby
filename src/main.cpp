#include <iostream>
#include <set>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include "lobbyserver.h"

int main() {
    try {
    Lobbyserver server_instance;

    // Start a thread to run the processing loop
    thread t(bind(&Lobbyserver::process_messages,&server_instance));
    server_instance.run(9002);
    t.join();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}
