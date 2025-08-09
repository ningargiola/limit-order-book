/**
 * @file main.cpp
 * @brief CLI driver for the Limit Order Book (LOB) matching engine.
 *
 * This program provides an interactive command-line interface to the OrderBook.
 * It supports manual commands (BUY, SELL, CANCEL, MODIFY, PRINT, TRADES, EXPORT_BOOK, EXPORT_TRADES),
 * as well as a synthetic benchmark mode (BENCH) for throughput testing.
 *
 * Orders are processed in price-time priority with support for partial fills.
 *
 * @author Nick Ingargiola
 * @date 2025-08-08
 */

 #include "order_book.h"
 #include <iostream>
 #include <sstream>
 #include <string>
 #include <chrono>
 #include <random>
 
 /**
  * @brief Program entry point.
  *
  * Reads commands from stdin until "EXIT" is received.
  * Commands can be manually entered or piped in from a file/stream.
  *
  * @return int Exit code (0 on success).
  */
 int main() {
     OrderBook book;
 
     // Directory for CSV exports
     book.setExportDir("exports");
 
     // Disable CSV auto-export for performance in benchmarks
     book.setAutoExport(false);
 
     long timestamp = 1;  ///< Logical timestamp for order sequencing
     int nextId = 1;      ///< Incremental order ID counter
 
     std::string input;
     while (std::getline(std::cin, input)) {
         if (input.empty())
             continue; // Ignore blank lines
 
         std::cout << ">" << input << "\n";
         std::istringstream iss(input);
         std::string command;
         iss >> command;
 
         // ------------------------------------------------
         // BUY / SELL: Create a new order
         // ------------------------------------------------
         if (command == "BUY" || command == "SELL") {
             double price;
             int qty;
             if (iss >> price >> qty) {
                 OrderType type = (command == "BUY") ? OrderType::BUY : OrderType::SELL;
                 book.addOrder(Order(nextId++, type, price, qty, timestamp++));
             }
         }
         // ------------------------------------------------
         // CANCEL: Remove an existing order by ID
         // ------------------------------------------------
         else if (command == "CANCEL") {
             int id;
             if (iss >> id)
                 std::cout << (book.cancelOrder(id) ? "Order cancelled.\n" : "Order not found.\n");
         }
         // ------------------------------------------------
         // MODIFY: Update price/quantity for an order by ID
         // ------------------------------------------------
         else if (command == "MODIFY") {
             int id, qty;
             double price;
             if (iss >> id >> qty >> price)
                 std::cout << (book.modifyOrder(id, qty, price, timestamp++) ? "Order modified.\n" : "Order not found.\n");
         }
         // ------------------------------------------------
         // PRINT: Display the current state of the order book
         // ------------------------------------------------
         else if (command == "PRINT") {
             book.printBook();
         }
         // ------------------------------------------------
         // TRADES: Display executed trades
         // ------------------------------------------------
         else if (command == "TRADES") {
             book.printTrades();
         }
         // ------------------------------------------------
         // EXPORT_BOOK: Save the current order book to CSV
         // ------------------------------------------------
         else if (command == "EXPORT_BOOK") {
             book.exportBookCSV();
         }
         // ------------------------------------------------
         // EXPORT_TRADES: Save executed trades to CSV
         // ------------------------------------------------
         else if (command == "EXPORT_TRADES") {
             book.exportTradesCSV();
         }
         // ------------------------------------------------
         // BENCH: Run synthetic benchmark
         // BENCH <numOrders>
         // ------------------------------------------------
         else if (command == "BENCH") {
             int numOrders;
             iss >> numOrders;
             if (numOrders <= 0) numOrders = 100000;
 
             // Fixed RNG seed for repeatability
             std::mt19937 rng(42);
             std::uniform_real_distribution<double> priceDist(90.0, 110.0);
             std::uniform_int_distribution<int> qtyDist(1, 5);
             std::uniform_int_distribution<int> sideDist(0, 1);
 
             auto start = std::chrono::high_resolution_clock::now();
 
             for (int i = 0; i < numOrders; i++) {
                 OrderType type = sideDist(rng) ? OrderType::BUY : OrderType::SELL;
                 double price = priceDist(rng);
                 int qty = qtyDist(rng);
                 book.addOrder(Order(nextId++, type, price, qty, timestamp++));
             }
 
             auto end = std::chrono::high_resolution_clock::now();
             std::chrono::duration<double> elapsed = end - start;
 
             double tradesPerSec = book.getTrades().size() / elapsed.count();
             std::cout << "\nBENCH RESULTS:\n";
             std::cout << "Orders processed: " << numOrders << "\n";
             std::cout << "Trades executed: " << book.getTrades().size() << "\n";
             std::cout << "Elapsed time: " << elapsed.count() << " sec\n";
             std::cout << "Throughput: " << tradesPerSec << " trades/sec\n";
         }
         // ------------------------------------------------
         // EXIT: End program
         // ------------------------------------------------
         else if (command == "EXIT") {
             break;
         }
         // ------------------------------------------------
         // UNKNOWN COMMAND
         // ------------------------------------------------
         else {
             std::cerr << "Unknown command: " << command << "\n";
         }
     }
 
     return 0;
 }
 