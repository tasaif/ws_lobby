#include "global.h"
#include "config.h"
#include "lobbyserver.h"

int main() {
    try {
      Config config("config.json");
      Lobbyserver server_instance(config.validation_url);
      // Start a thread to run the processing loop
      thread t(bind(&Lobbyserver::process_messages,&server_instance));
      server_instance.run(config.port);
      t.join();
    } catch (websocketpp::exception const & e) {
      std::cout << e.what() << std::endl;
    }
}
