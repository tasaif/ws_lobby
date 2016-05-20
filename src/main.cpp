#include "global.h"
#include "lobbyserver.h"

int main() {
    try {
      Lobbyserver server_instance;
      // Start a thread to run the processing loop
      thread t(bind(&Lobbyserver::process_messages,&server_instance));
      server_instance.run(8081);
      t.join();
    } catch (websocketpp::exception const & e) {
      std::cout << e.what() << std::endl;
    }
}
