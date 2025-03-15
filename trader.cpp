#include <algorithm>
#include <iostream>
#include <thread>

#include "DeribitAPI.hpp"

int main() {
  std::string client_id = std::getenv("DERIBIT_CLIENT_ID") ? std::getenv("DERIBIT_CLIENT_ID") : "";
  std::string client_secret = std::getenv("DERIBIT_CLIENT_SECRET") ? std::getenv("DERIBIT_CLIENT_SECRET") : "";

  DeribitAPI deribit;
  if (!deribit.authenticate(client_id, client_secret)) {
    return 1;
  }

  std::string market_type;  // futures or options
  while (true) {
    std::cout << "----Choose market type (futures/options): ";
    std::cin >> market_type;
    std::transform(market_type.begin(), market_type.end(), market_type.begin(), ::tolower);

    if (market_type == "futures" || market_type == "options") {
      break;
    }
    std::cerr << "----Invalid market type! Please enter 'futures', or 'options'.\n";
  }

  std::string instrument;
  std::cout << "----Enter instrument: ";
  std::cin >> instrument;

  // GET ORDERBOOK
  std::cout << "----Orderbook: \n";
  if (!deribit.get_orderbook(instrument)) {
    return 1;
  }

  // PLACING ORDER
  char place_order;
  std::cout << "----Do you want to place an order? (y/n): ";
  std::cin >> place_order;
    
  /* ----------- Benchmarking ----------- */
  // auto trading_start = std::chrono::high_resolution_clock::now();

  std::string order_id;
  std::string order_type; // market or limit
  if (place_order == 'y' || place_order == 'Y') {
    std::string side;  // buy or sell
    
    while (true) {
      std::cout << "----Enter order side (buy/sell): ";
      std::cin >> side;
      std::transform(side.begin(), side.end(), side.begin(), ::tolower);

      if (side == "buy" || side == "sell") {
        break;
      }
      std::cerr << "----Invalid order side! Please enter 'buy' or 'sell'.\n";
    }

    // Input order type
    while (true) {
      std::cout << "----Enter order type (market/limit): ";
      std::cin >> order_type;
      std::transform(order_type.begin(), order_type.end(), order_type.begin(), ::tolower);

      if (order_type == "market" || order_type == "limit") {
        break;
      }
      std::cerr << "----Invalid order type! Please enter 'market' or 'limit'.\n";
    }

    int amount;
    std::cout << "----Enter order amount: ";
    std::cin >> amount;

    double price = 0;  // not needed for market orders
    if (order_type == "limit") {
      std::cout << "----Enter order price: ";
      std::cin >> price;
    }

    order_id = deribit.place_order(instrument, amount, side, "limit", price);

    /* ----------- Benchmarking ----------- */
    // auto trading_end = std::chrono::high_resolution_clock::now();
    // auto trading_latency = std::chrono::duration<double, std::milli>(trading_end - trading_start).count();
    // std::cout << "\n----Total Trading Loop Latency: " << trading_latency << " ms\n";

  }

  // MODIFY ORDER
  if (!order_id.empty()) {
    char modify_order;
    std::cout << "----Do you want to modify the order? (y/n): ";
    std::cin >> modify_order;

    if (modify_order == 'y' || modify_order == 'Y') {
      if (order_type == "market") {
        std::cerr << "----Cannot modify market orders! They are filled instantly.\n";
      } else {
        double new_price;
        int new_amount;

        std::cout << "----Enter new amount: ";
        std::cin >> new_amount;

        std::cout << "----Enter new price: ";
        std::cin >> new_price;

        deribit.modify_order(order_id, new_amount, new_price);
      }
    }
  }

  // CANCEL ORDER
  if (!order_id.empty()) {
    char cancel_order;
    std::cout << "----Do you want to cancel the order? (y/n): ";
    std::cin >> cancel_order;

    if (cancel_order == 'y' || cancel_order == 'Y') {
      deribit.cancel_order(order_id);
    }
  }

  // GET POSITIONS
  std::string asset;
  std::cout << "----Enter asset for position retrieval (BTC, ETH, etc.): ";
  std::cin >> asset;
  deribit.get_positions(asset);
  return 0;
}
