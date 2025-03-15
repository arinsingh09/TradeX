#include "WebSocketClient.hpp"

namespace websocket = boost::beast::websocket;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

WebSocketClient::WebSocketClient()
    : ctx(boost::asio::ssl::context::tlsv12_client),
      resolver(ioc),
      ws(ioc, ctx) {}

void WebSocketClient::connect(const std::string& uri) {
  try {
    auto const host = "test.deribit.com";
    auto const port = "443";
    auto const path = "/ws/api/v2";

    // Resolve the host
    auto const results = resolver.resolve(host, port);
    boost::asio::connect(ws.next_layer().next_layer(), results.begin(), results.end());

    // Perform SSL handshake
    ws.next_layer().handshake(boost::asio::ssl::stream_base::client);

    // Perform WebSocket handshake
    ws.handshake(host, path);

    std::cout << "✅ WebSocket Connected to " << uri << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "❌ Connection Error: " << e.what() << std::endl;
  }
}

void WebSocketClient::authenticate(const std::string& client_id, const std::string& client_secret) {
  std::string auth_msg = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "public/auth",
        "params": {
        "grant_type": "client_credentials",
        "client_id": ")" + client_id + R"(",
        "client_secret": ")" + client_secret + R"("
        }
        })";

  ws.write(boost::asio::buffer(auth_msg));
  std::cout << "🔑 Authenticating WebSocket..." << std::endl;

  boost::beast::flat_buffer buffer;
  ws.read(buffer);
  std::string response_str = boost::beast::buffers_to_string(buffer.data());
  buffer.clear();

  try {
    json response_json = json::parse(response_str);
    if (response_json.contains("result") && response_json["result"].contains("access_token")) {
      std::string access_token = response_json["result"]["access_token"];
      std::cout << "✅ Authentication successful!\n";
      // std::cout << "✅ Authentication successful! Access Token: " << access_token << std::endl;
    } else {
      std::cerr << "❌ Authentication failed! Response: " << response_str << std::endl;
    }
  } catch (const json::parse_error& e) {
    std::cerr << "❌ JSON Parsing Error in authentication: " << e.what() << std::endl;
    std::cerr << "Raw response: " << response_str << std::endl;
  }
}

void WebSocketClient::subscribe(const std::string& channel) {
    std::string subscribe_msg = R"({
        "jsonrpc": "2.0",
        "id": 42,
        "method": "public/subscribe",
        "params": {
            "channels": [")" + channel + R"("]
        }
    })";

    ws.async_write(boost::asio::buffer(subscribe_msg), 
        [&](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "❌ WebSocket Write Error: " << ec.message() << std::endl;
            } else {
                std::cout << "📡 Subscribing to: " << channel << std::endl;
            }
        });

}

void WebSocketClient::listen() {
    auto buffer = std::make_shared<boost::beast::flat_buffer>();

    std::function<void(boost::system::error_code, std::size_t)> read_handler;

    read_handler = [&](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "❌ WebSocket Read Error: " << ec.message() << "\n";
            return;
        }

        /* ----------- Benchmarking network response ----------- */
        // auto start_time = std::chrono::high_resolution_clock::now();
        std::string raw_update = boost::beast::buffers_to_string(buffer->data());
        buffer->consume(buffer->size()); // Clear buffer after processing
        // auto network_end = std::chrono::high_resolution_clock::now();

        try {
            /* ----------- Benchmarking json parsing ----------- */
            // auto json_start = std::chrono::high_resolution_clock::now();
            json json_update = json::parse(std::move(raw_update));
            // auto json_end = std::chrono::high_resolution_clock::now();


            /* double network_latency = std::chrono::duration<double, std::milli>(network_end - start_time).count();
            double json_latency = std::chrono::duration<double, std::milli>(json_end - json_start).count();
            double total_latency = std::chrono::duration<double, std::milli>(json_end - start_time).count();

            std::cout << "\n📡 Market Data Latency: " << total_latency << " ms (Network: "
                      << network_latency << " ms, JSON Parsing: " << json_latency << " ms)\n"; */


            // Ignore first subscription response
            if (json_update.contains("result") && json_update["result"].is_array()) {
                ws.async_read(*buffer, read_handler);
                return;
            }

            // Process market updates
            if (json_update.contains("params") && json_update["params"].contains("data")) {
                auto data = json_update["params"]["data"];
                std::string instrument = data.value("instrument_name", "Unknown");
                std::string update_type = data.value("type", ""); // "snapshot" or "change"

                if (update_type == "snapshot") {
                    std::cout << "\n📡 [Market Snapshot] " << instrument << "\n";
                    bid_book.clear();
                    ask_book.clear();
                }

                // Update bid & ask books
                update_order_book(data["bids"], bid_book);
                update_order_book(data["asks"], ask_book);

                // Display order book summary
                display_order_book();
            } else {
                std::cout << "📌 Unrecognized Market Update: " << raw_update << "\n";
            }
        } catch (const json::parse_error& e) {
            std::cerr << "❌ WebSocket Parsing Error: " << e.what() << "\n";
            std::cerr << "Raw update: " << raw_update << "\n";
        }

        // Read next message
        ws.async_read(*buffer, read_handler);
    };

    ws.async_read(*buffer, read_handler);
    ioc.run();
}

// Helper function for listen()
void WebSocketClient::display_order_book() {
    if (bid_book.empty() || ask_book.empty()) return;

    auto best_bid = --bid_book.end();
    auto best_ask = ask_book.begin();
    double spread = best_ask->first - best_bid->first;

    double total_bid_size = std::accumulate(bid_book.begin(), bid_book.end(), 0.0, 
                                            [](double sum, const auto& pair) { return sum + pair.second; });

    double total_ask_size = std::accumulate(ask_book.begin(), ask_book.end(), 0.0, 
                                            [](double sum, const auto& pair) { return sum + pair.second; });

    std::cout << std::fixed << std::setprecision(8);
    std::cout << "   🟢 Best Bid: " << best_bid->second << " @ " << best_bid->first << "\n";
    std::cout << "   🔴 Best Ask: " << best_ask->second << " @ " << best_ask->first << "\n";
    std::cout << "   📏 Spread: " << spread << "\n";
    std::cout << "   📊 Total Bid Depth: " << total_bid_size << "\n";
    std::cout << "   📊 Total Ask Depth: " << total_ask_size << "\n";
}

// Helper function for listen()
void WebSocketClient::update_order_book(const json& orders, std::map<double, double>& book) {
    for (const auto& order : orders) {
        double price = order[1].get<double>();
        double size = order[2].get<double>();
        if (size > 0) {
            book[price] = size;
        } else {
            book.erase(price);
        }
    }
}