#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <unordered_map>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

class WebSocketServer {
 public:
  WebSocketServer(boost::asio::io_context& ioc, short port);
  void run();

 private:
  void accept();
  void on_accept(boost::system::error_code ec, tcp::socket socket);
  void handle_client(std::shared_ptr<websocket::stream<tcp::socket>> ws);
  void process_message(std::shared_ptr<websocket::stream<tcp::socket>> ws, const std::string& msg);

  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::unordered_map<std::string, std::set<std::shared_ptr<websocket::stream<tcp::socket>>>> subscribers_;
};

#endif  // WEBSOCKET_SERVER_HPP
