#include <iostream>
#include <thread>
#include "WebSocketClient.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error! Usage: ./realtime_client <PORT NUMBER> <INSTRUMENT>";
        return 1;
    }

    const std::string PORT = argv[1];
    std::string instrument = argv[2];

    std::string client_id = std::getenv("DERIBIT_CLIENT_ID") ? std::getenv("DERIBIT_CLIENT_ID") : "";
    std::string client_secret = std::getenv("DERIBIT_CLIENT_SECRET") ? std::getenv("DERIBIT_CLIENT_SECRET") : "";

    WebSocketClient client;
    
    client.connect("ws://localhost:" + PORT);
    client.authenticate(client_id, client_secret);
    client.subscribe("book." + instrument + ".raw");
    std::cout << "----\nWebSocket streaming started for " << instrument << "...\n";
    client.listen();

    return 0;
}
