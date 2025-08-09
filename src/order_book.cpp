/**
 * @file order_book.cpp
 * @brief Implementation of the OrderBook class for a simple high-performance matching engine.
 *
 * Handles adding, modifying, and canceling orders, as well as executing trades
 * based on price-time priority. Supports CSV exports and optional live auto-exporting.
 *
 * @author Nick Ingargiola
 * @date 2025-08-08
 */

 #include "order_book.h"
 #include <iostream>
 #include <algorithm>
 #include <optional>
 #include <fstream>
 #include <chrono>
 #include <iomanip>
 #include <sstream>
 #include <ctime>
 #include <filesystem>
 #include <atomic>
 
 /// Global atomic counter to generate unique snapshot file names.
 static std::atomic<uint64_t> g_snapshot{0};
 
 /**
  * @brief Generate a unique, timestamped filename for exports.
  *
  * @param dir Directory where file will be saved.
  * @param baseName Base filename (e.g., "trades" or "book").
  * @return Full path string to the generated file.
  */
 static std::string makeTimestampedFilename(const std::string &dir, const std::string &baseName)
 {
     namespace fs = std::filesystem;
 
     fs::path outDir(dir.empty() ? "." : dir);
     std::error_code ec;
     fs::create_directories(outDir, ec);
 
     auto now = std::chrono::system_clock::now();
     std::time_t t = std::chrono::system_clock::to_time_t(now);
     std::tm tm{};
 #ifdef _WIN32
     localtime_s(&tm, &t);
 #else
     localtime_r(&t, &tm);
 #endif
     uint64_t seq = ++g_snapshot;
 
     std::ostringstream oss;
     oss << baseName << "_"
         << std::put_time(&tm, "%Y%m%d_%H%M%S")
         << "_" << seq
         << ".csv";
     return (outDir / oss.str()).string();
 }
 
 void OrderBook::setExportDir(const std::string &dir)
 {
     exportDir = dir.empty() ? "." : dir;
 
     std::error_code ec;
     std::filesystem::create_directories(exportDir, ec); // Create folder if missing
     if (ec)
     {
         std::cerr << "Warning: Could not create export directory: " << exportDir << "\n";
     }
 }
 
 void OrderBook::addOrder(const Order &order)
 {
     // Validation: ignore invalid orders
     if (order.quantity <= 0)
     {
         std::cerr << "Error: Order quantity must be positive.\n";
         return;
     }
 
     // Insert into correct side (bids or asks) maintaining list order
     if (order.type == OrderType::BUY)
     {
         if (bids.empty() || order.price > bids.front().price)
             bids.push_front(order), orderIndex[order.id] = {OrderType::BUY, bids.begin()};
         else
             bids.push_back(order), orderIndex[order.id] = {OrderType::BUY, std::prev(bids.end())};
     }
     else
     {
         if (asks.empty() || order.price < asks.front().price)
             asks.push_front(order), orderIndex[order.id] = {OrderType::SELL, asks.begin()};
         else
             asks.push_back(order), orderIndex[order.id] = {OrderType::SELL, std::prev(asks.end())};
     }
 
     // Attempt matching after adding
     matchOrders();
 }
 
 bool OrderBook::modifyOrder(int id, int newQty, double newPrice, long newTimestamp)
 {
     auto it = orderIndex.find(id);
     if (it == orderIndex.end())
         return false;
 
     OrderType type = it->second.type;
 
     // Remove and reinsert with new parameters
     cancelOrder(id);
     addOrder(Order(id, type, newPrice, newQty, newTimestamp));
     return true;
 }
 
 bool OrderBook::cancelOrder(int id)
 {
     auto it = orderIndex.find(id);
     if (it == orderIndex.end())
         return false;
 
     auto [type, orderIt] = it->second;
     if (type == OrderType::BUY)
         bids.erase(orderIt);
     else
         asks.erase(orderIt);
 
     orderIndex.erase(it);
     return true;
 }
 
 void OrderBook::matchOrders()
 {
     // Continue matching as long as best bid >= best ask
     while (!bids.empty() && !asks.empty() && bids.front().price >= asks.front().price)
     {
         Order &buy = bids.front();
         Order &sell = asks.front();
 
         executeTrade(buy, sell);
 
         if (buy.quantity == 0)
         {
             orderIndex.erase(buy.id);
             bids.pop_front();
         }
         if (sell.quantity == 0)
         {            
             orderIndex.erase(sell.id);
             asks.pop_front();
         }
     }
 }
 
 void OrderBook::executeTrade(Order &buy, Order &sell)
 {
     int qty = std::min(buy.quantity, sell.quantity);
     double px = sell.price;
     long t = std::max(buy.timestamp, sell.timestamp);
 
     trades.emplace_back(buy.id, sell.id, px, qty, t);
 
     buy.quantity -= qty;
     sell.quantity -= qty;
     totalVolumeTraded += qty;
 
     if (autoExport)
     {
         exportTradesCSV();
         exportBookCSV();
     }
 }
 
 void OrderBook::printTrades() const
 {
     std::cout << "\nTrades:\n";
     for (const auto &trade : trades)
     {
         std::cout << "Buy ID: " << trade.buyId
                   << ", Sell ID: " << trade.sellId
                   << ", Price: $" << trade.price
                   << ", Quantity: " << trade.quantity
                   << ", Timestamp: " << trade.timestamp << "\n";
     }
 }
 
 void OrderBook::printBook() const
 {
     std::cout << "\nOrder Book:\n";
 
     std::cout << "BIDS:\n";
     if (!bids.empty()) {
         double current = bids.front().price;
         int count = 0;
         int totalQty = 0;
         for (const auto& o : bids) {
             if (o.price != current) {
                 std::cout << " $" << current << " x " << count << " orders, totalQty=" << totalQty << "\n";
                 current = o.price; count = 0; totalQty = 0;
             }
             ++count; totalQty += o.quantity;
         }
         std::cout << " $" << current << " x " << count << " orders, totalQty=" << totalQty << "\n";
     }
 
     std::cout << "ASKS:\n";
     if (!asks.empty()) {
         double current = asks.front().price;
         int count = 0;
         int totalQty = 0;
         for (const auto& o : asks) {
             if (o.price != current) {
                 std::cout << " $" << current << " x " << count << " orders, totalQty=" << totalQty << "\n";
                 current = o.price; count = 0; totalQty = 0;
             }
             ++count; totalQty += o.quantity;
         }
         std::cout << " $" << current << " x " << count << " orders, totalQty=" << totalQty << "\n";
     }
 
     std::cout << "Total Volume Traded: " << totalVolumeTraded << " units\n";
 }
 
 void OrderBook::exportTradesCSV(const std::string &baseName) const
 {
     std::string filename = makeTimestampedFilename(exportDir, baseName);
 
     std::ofstream out(filename);
     if (!out.is_open())
     {
         std::cerr << "Error: Could not open file " << filename << "\n";
         return;
     }
 
     out << "timestamp,buyId,sellId,price,quantity\n";
     for (const auto &trade : trades)
     {
         out << trade.timestamp << ","
             << trade.buyId << ","
             << trade.sellId << ","
             << trade.price << ","
             << trade.quantity << "\n";
     }
 
     std::cout << "Trades exported to " << filename << "\n";
 }
 
 void OrderBook::exportBookCSV(const std::string &baseName) const
 {
     std::string filename = makeTimestampedFilename(exportDir, baseName);
     std::ofstream out(filename);
     if (!out.is_open()) {
         std::cerr << "Error: Could not open file " << filename << "\n";
         return;
     }
 
     out << "side,price,quantity,id,timestamp\n";
 
     for (const auto& o : asks)
         out << "SELL," << o.price << "," << o.quantity << "," << o.id << "," << o.timestamp << "\n";
 
     for (const auto& o : bids)
         out << "BUY," << o.price << "," << o.quantity << "," << o.id << "," << o.timestamp << "\n";
 
     std::cout << "Order book exported to " << filename << "\n";
 }
 