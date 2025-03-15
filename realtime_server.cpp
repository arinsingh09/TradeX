#include <boost/asio/io_context.hpp>
#include <iostream>

#include "WebSocketServer.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error! Usage: ./realtime_server <PORT NUMBER>";
    return 1;
  }

  const short PORT = std::stoi(argv[1]);

  boost::asio::io_context ioc;
  WebSocketServer server(ioc, PORT);

  std::cout << "WebSocket Server started on port: " << PORT << "\n";

  server.run();

  return 0;
}
