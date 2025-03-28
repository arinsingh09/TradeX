# TradeX - Real-Time Trading System

## Overview
TradeX is a real-time trading system developed in C++ that integrates with the **Deribit API** for order execution and market data streaming. It enables placing, modifying, and canceling orders while streaming live order book updates using WebSockets.

## Features

- **Order Management (REST API):**
  - Place, modify, and cancel market/limit orders.
  - Fetch order book and active positions.
- **Real-Time Market Data Streaming (WebSockets):**
  - Subscribe to live order book updates.
  - Stream bid/ask prices, depths and spreads.
- **Trading Engine:**
  - Executes trading operations using REST API requests.
  - Provides command-line interaction for traders.
- **WebSocket Client & Server:**
  - Client subscribes to market data and streams price updates.
  - Server handles multiple connections and distributes order book updates.

## Tech Stack

- **Languages:** C++
- **Networking:** WebSockets, REST API
- **Libraries:** C++ Standard Library, JSON Parser

## Installation & Setup

1. Clone the repository:
   ```zsh
   git clone https://github.com/arinsingh09/TradeX.git
   cd TradeX
   ```
2. Install dependencies and build:
   ```zsh
   mkdir build && cd build
   cmake ..
   make --build . --parallel #on macOS
   ```
3. Set up API credentials:
   ```zsh
   export DERIBIT_CLIENT_ID=your_client_id
   export DERIBIT_CLIENT_SECRET=your_client_secret
   ```
4. Run the WebSocket server:
   ```zsh
   ./realtime_server <PORT NUMBER>
   ```
5. Run the WebSocket client:
   ```zsh
   ./realtime_client <PORT NUMBER> <INSTRUMENT>
   ```
6. Start trading using the command-line interface:
   ```zsh
   ./trader
   ```

## Contribution

Contributions are welcome! Feel free to submit pull requests or report issues.
