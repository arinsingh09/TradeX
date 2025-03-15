#include "WebSocketServer.hpp"

using json = nlohmann::json;

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc, short port)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), socket_(ioc) {}

void WebSocketServer::run() { accept(); }

void WebSocketServer::accept() {
  auto new_socket = std::make_shared<tcp::socket>(acceptor_.get_executor());

  acceptor_.async_accept(*new_socket, [&](boost::system::error_code ec) {
    on_accept(ec, std::move(*new_socket));
  });
}

void WebSocketServer::on_accept(boost::system::error_code ec, tcp::socket socket) {
  if (ec) return;
  auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
  ws->async_accept([&](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "----WebSocket handshake successful!\n";
      handle_client(ws);
    } else {
      std::cerr << "----WebSocket handshake failed: " << ec.message() << "\n";
    }
    accept();
  });
}

void WebSocketServer::handle_client(
    std::shared_ptr<websocket::stream<tcp::socket>> ws) {
  auto buffer = std::make_shared<boost::beast::flat_buffer>();

  ws->async_read(*buffer, [&](boost::system::error_code ec, std::size_t) {
    if (!ec && ws->is_open()) {
      std::string msg = boost::beast::buffers_to_string(buffer->data());
      process_message(ws, msg);

      // Next Message
      handle_client(ws);
    } else {
      std::cerr << "----Client disconnected.\n";

      // Remove client from all subscriptions
      for (auto& [instrument, clients] : subscribers_) {
        clients.erase(ws);
      }
    }
  });
}

void WebSocketServer::process_message(std::shared_ptr<websocket::stream<tcp::socket>> ws, const std::string& msg) {
  try {
    json request = json::parse(msg);
    if (request.contains("subscribe")) {
      std::string instrument = request["subscribe"];
      subscribers_[instrument].emplace(ws);
      std::cout << "----Client subscribed to: " << instrument << "\n";
    } else if (request.contains("unsubscribe")) {
      std::string instrument = request["unsubscribe"];
      subscribers_[instrument].erase(ws);
      std::cout << "----Client unsubscribed from: " << instrument << "\n";
    }
  } catch (json::parse_error&) {
    std::cerr << "----Invalid JSON received: " << msg << "\n";
  }
}