#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

#include "WebSocketServer.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class WebSocketClient {
 public:
  WebSocketClient();
  void authenticate(const std::string& client_id, const std::string& client_secret);
  void connect(const std::string& uri);
  void subscribe(const std::string& channel);
  void listen();
  void display_order_book();
  void update_order_book(const json& orders, std::map<double, double>& book);

 private:
  boost::asio::io_context ioc;
  boost::asio::ssl::context ctx;
  boost::asio::ip::tcp::resolver resolver;
  boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws;

  std::map<double, double> bid_book;
  std::map<double, double> ask_book;
};

#endif  // WEBSOCKET_CLIENT_HPP
