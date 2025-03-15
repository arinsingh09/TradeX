#ifndef DERIBIT_API_HPP
#define DERIBIT_API_HPP

#include <iostream>
#include <string>

#include "HttpClient.hpp"
#include "nlohmann/json.hpp"

class DeribitAPI {
 private:
  std::string access_token;
  HttpClient http_client;

 public:
  DeribitAPI();

  bool authenticate(const std::string& client_id, const std::string& client_secret);
  bool get_orderbook(const std::string& instrument);
  std::string place_order(const std::string& instrument, int amount, const std::string& side, const std::string& type, double price = 0.0);
  bool cancel_order(const std::string& order_id);
  bool modify_order(const std::string& order_id, int new_amount, double new_price);
  bool get_positions(const std::string& currency);
};

#endif
