#include "DeribitAPI.hpp"

#include <chrono>

using json = nlohmann::json;

DeribitAPI::DeribitAPI() {}

// Authenticate and store the token
bool DeribitAPI::authenticate(const std::string& client_id,
                              const std::string& client_secret) {
  std::string url = "https://test.deribit.com/api/v2/public/auth";
  std::string post_data = R"({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "public/auth",
        "params": {
            "grant_type": "client_credentials",
            "client_id": ")" +
                          client_id + R"(",
            "client_secret": ")" +
                          client_secret + R"("
        }
    })";

  std::string response = http_client.send_request(url, post_data);

  try {
    auto json_response = json::parse(response);
    if (json_response.contains("result") &&
        json_response["result"].contains("access_token")) {
      access_token = json_response["result"]["access_token"];
      std::cout << "Authentication successful!\n";
      // std::cout << "DEBUG: Stored Access Token: " << access_token << "\n";
      return true;
    } else {
      std::cerr << "Authentication failed: " << json_response["error"]["message"] << std::endl;
      return false;
    }
  } catch (const std::exception& e) {
    std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
    return false;
  }
}

// View orderbook
bool DeribitAPI::get_orderbook(const std::string& instrument) {
  std::string url = "https://test.deribit.com/api/v2/public/get_order_book";

  std::string post_data = R"({
          "jsonrpc": "2.0",
          "id": 42,
          "method": "public/get_order_book",
          "params": {
              "instrument_name": ")" +
                              instrument + R"("
          }
      })";

  std::string response = http_client.send_request(url, post_data);
  // std::cerr << "DEBUG: " << response << "\n";
  try {
    json json_response = json::parse(response);
    if (!json_response.contains("result")) {
      std::cerr << "Error: Invalid order book response!\n";
      return false;
    }

    auto bids = json_response["result"]["bids"];
    auto asks = json_response["result"]["asks"];

    std::cout << "Order Book: " << instrument << ":\n";
    std::cout << "────────────────────────────────────────────────────────────\n";
    std::cout << "   🟢 Bids (Buyers)         🔴 Asks (Sellers)\n";
    std::cout << "────────────────────────────────────────────────────────────\n";

    // Display top 5 bid/ask prices
    for (size_t i = 0; i < std::min(bids.size(), asks.size()) && i < 5; ++i) {
      std::cout << "   " << std::setw(8) << bids[i][1] << " @ " << std::setw(10)
                << bids[i][0] << "      " << std::setw(8) << asks[i][1] << " @ "
                << std::setw(10) << asks[i][0] << "\n";
    }
    std::cout << "────────────────────────────────────────────────────────────\n";

    return true;
  } catch (const json::parse_error& e) {
    std::cerr << "JSON Parsing Error: " << e.what() << "\n";
    return false;
  }
}

// Place an order
std::string DeribitAPI::place_order(const std::string& instrument, int amount,
                                    const std::string& side,
                                    const std::string& type, double price) {
  // auto start_time = std::chrono::high_resolution_clock::now();

  std::string url = "https://test.deribit.com/api/v2/private/" + side;

  std::string post_data = R"({
              "jsonrpc": "2.0",
              "id": 42,
              "method": "private/)" +
              side + R"(",
              "params": {
                  "instrument_name": ")" +
              instrument + R"(",
                  "amount": )" +
              std::to_string(amount) + R"(,
                  "type": ")" +
              type + R"("
            )";

  if (type == "limit") {
    post_data += R"(, "price": )" + std::to_string(price);
  }

  post_data += R"( } })";

  /* ----------- Benchmarking http request/response ----------- */
  // auto http_start = std::chrono::high_resolution_clock::now();
  std::string response = http_client.send_request(url, post_data, access_token);
  // auto http_end = std::chrono::high_resolution_clock::now();

  /* ----------- Benchmarking json parsing ----------- */
  // auto json_start = std::chrono::high_resolution_clock::now();
  json order_json = json::parse(response);
  // auto json_end = std::chrono::high_resolution_clock::now();

/*   auto total_latency =
      std::chrono::duration<double, std::milli>(json_end - start_time).count();
  auto network_latency = std::chrono::duration<double, std::milli>(json_start - start_time).count();
  auto json_parsing_latency = std::chrono::duration<double, std::milli>(json_end - json_start).count();

  std::cout << "\n⏱️ Order Placement Latency: " << total_latency << " ms (Network: " << network_latency
            << " ms, JSON Parsing: " << json_parsing_latency << " ms)\n"; */

  if (order_json.contains("error")) {
    std::cerr << "❌ Order Failed: " << order_json["error"]["message"] << "\n";

    if (order_json["error"].contains("data")) {
      auto error_data = order_json["error"]["data"];
      if (error_data.contains("param") && error_data.contains("reason")) {
        std::cerr << "🔹 Reason: '" << error_data["param"] << "' - " << error_data["reason"] << "\n";
      }
    }
  } else if (order_json.contains("result") && order_json["result"].contains("order")) {
    auto order = order_json["result"]["order"];
    std::cout << "✅ Order Placed! " << "\n";
    std::cout << "📌 Instrument: " << order["instrument_name"] << "\n";
    std::cout << "🛒 Amount: " << order["amount"] << "\n";
    std::cout << "💰 Price: " << (type == "market" ? "Market Price" : std::to_string(order["price"].get<double>()))<< "\n";
    std::cout << "🔄 Order State: " << order["order_state"] << "\n";
    std::cout << "🆔 Order ID: " << order["order_id"] << "\n";

    return order["order_id"];
  } else {
    std::cerr << "❌ Order Failed: Unknown error. Response: " << order_json.dump(2) << "\n";
  }

  return "";
}

// Modify an order
bool DeribitAPI::modify_order(const std::string& order_id, int new_amount,
                              double new_price) {
  std::string url = "https://test.deribit.com/api/v2/private/edit";

  std::string post_data = R"({
        "jsonrpc": "2.0",
        "id": 42,
        "method": "private/edit",
        "params": {
            "order_id": ")" +
                          order_id + R"(",
            "amount": )" + std::to_string(new_amount) +
                          R"(,
            "price": )" + std::to_string(new_price) +
                          R"(
        }
    })";

  // std::cout << "DEBUG: Modifying Order ID: " << order_id << " to Price: " <<
  // new_price << " and Amount: " << new_amount << "\n";

  std::string response = http_client.send_request(url, post_data, access_token);

  json json_response = json::parse(response);

  if (json_response.contains("error")) {
    std::cerr << "❌ Order Modify Failed: " << json_response["error"]["message"] << "\n";
    return false;
  }

  std::cout << "✅ Order Modified!\n";
  std::cout << "🔄 New Price: " << json_response["result"]["order"]["price"] << "\n";
  std::cout << "🆔 Order ID: " << json_response["result"]["order"]["order_id"] << "\n";

  return true;
}

// Cancel an order
bool DeribitAPI::cancel_order(const std::string& order_id) {
  std::string url = "https://test.deribit.com/api/v2/private/cancel";

  std::string post_data = R"({
        "jsonrpc": "2.0",
        "id": 42,
        "method": "private/cancel",
        "params": {
            "order_id": ")" +
                          order_id + R"("
        }
    })";

  // std::cout << "DEBUG: Cancelling Order ID: " << order_id << std::endl;
  std::string response = http_client.send_request(url, post_data, access_token);

  json json_response = json::parse(response);

  if (json_response.contains("error")) {
    std::cerr << "❌ Cancel Failed: " << json_response["error"]["message"] << "\n";
    return false;
  }

  std::cout << "✅ Order Cancelled!\n";
  std::cout << "🆔 Order ID: " << json_response["result"]["order_id"] << "\n";
  std::cout << "❌ Reason: " << json_response["result"]["cancel_reason"] << "\n";

  return true;
}

// View positions
bool DeribitAPI::get_positions(const std::string& currency) {
  std::string url = "https://test.deribit.com/api/v2/private/get_positions";

  std::string post_data = R"({
      "jsonrpc": "2.0",
      "id": 42,
      "method": "private/get_positions",
      "params": {
          "currency": ")" +
                          currency + R"("
      }
  })";

  std::string response = http_client.send_request(url, post_data, access_token);

  try {
    json json_response = json::parse(response);
    if (!json_response.contains("result") || json_response["result"].empty()) {
      std::cerr << "❌ Error: No positions found for " << currency << "!\n";
      return false;
    }

    std::cout << "📊 " << currency << " Positions\n";
    std::cout << "────────────────────────────────────────────────────────────────────\n";

    for (const auto& position : json_response["result"]) {
      double size = position["size"].get<double>();
      double avg_price = position["average_price"].get<double>();
      std::string direction = position["direction"].get<std::string>();  // "buy", "sell", or "zero"
      std::string instrument = position["instrument_name"].get<std::string>();
      int leverage = position.contains("leverage") ? position["leverage"].get<int>() : 0;
      double liquidation_price = position.contains("estimated_liquidation_price") &&
                                !position["estimated_liquidation_price"].is_null() ? position["estimated_liquidation_price"].get<double>() : 0.0;

      std::cout << "📌 Instrument: " << instrument << "\n";
      std::cout << "📌 Size: " << std::setw(6) << size << "\n";
      std::cout << "💰 Avg. Entry Price: " << std::setw(8) << avg_price << "\n";

      if (size == 0) {
        std::cout << "📈 Direction: ⚪ None\n";
      } else if (direction == "buy") {
        std::cout << "📈 Direction: 🔼 Long\n";
      } else if (direction == "sell") {
        std::cout << "📉 Direction: 🔽 Short\n";
      }

      if (leverage > 0) {
        std::cout << "🔥 Leverage: " << leverage << "x\n";
      }

      if (liquidation_price > 0) {
        std::cout << "🏴 Est. Liquidation Price: " << liquidation_price << "\n";
      }

      std::cout << "────────────────────────────────────────────────────────────────────\n";
    }

    return true;
  } catch (const json::parse_error& e) {
    std::cerr << "❌ JSON Parsing Error: " << e.what() << "\n";
  }
  return false;
}
