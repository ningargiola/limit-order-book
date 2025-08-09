/**
 * @file stress.cpp
 * @brief High-volume stress test for the Limit Order Book.
 * 
 * Generates a large number of synthetic BUY/SELL orders with random prices and quantities,
 * then measures throughput (trades per second).
 * 
 * Usage:
 *   make stress-run                # Default: 2,000,000 orders
 *   make stress-run ORDERS=5000000 # Custom order count
 * 
 * This test is useful for performance tuning and benchmarking changes to the matching engine.
 * 
 * Author: Nick Ingargiola
 */

 #include "order_book.h"
 #include "order.h"
 #include <chrono>
 #include <iostream>
 #include <random>
 
 /**
  * @brief Entry point for the stress test.
  * 
  * @param argc Command-line arg count
  * @param argv argv[1] optionally sets number of orders to generate
  * @return int Exit status code
  */
 int main(int argc, char** argv) {
     OrderBook book;
     book.setAutoExport(false); // Disable CSV writes for speed
     book.setExportDir("exports");
 
     long timestamp = 1;  ///< Simulated timestamp
     int nextId = 1;      ///< Incrementing order ID
 
     // Default to 2M orders unless overridden by command-line
     int numOrders = 2'000'000;
     if (argc > 1) {
         numOrders = std::stoi(argv[1]);
     }
 
     // RNG setup for reproducible runs
     std::mt19937 rng(42);
     std::uniform_real_distribution<double> priceDist(90.0, 110.0); // Price range
     std::uniform_int_distribution<int> qtyDist(1, 5);               // Qty per order
     std::uniform_int_distribution<int> sideDist(0, 1);               // Buy or Sell
 
     auto start = std::chrono::high_resolution_clock::now();
 
     // Generate and submit orders
     for (int i = 0; i < numOrders; i++) {
         OrderType type = sideDist(rng) ? OrderType::BUY : OrderType::SELL;
         double price = priceDist(rng);
         int qty = qtyDist(rng);
         book.addOrder(Order(nextId++, type, price, qty, timestamp++));
     }
 
     auto end = std::chrono::high_resolution_clock::now();
     std::chrono::duration<double> elapsed = end - start;
 
     // Throughput calculation
     double tradesPerSec = book.getTrades().size() / elapsed.count();
 
     // Summary report
     std::cout << "STRESS RESULTS:\n";
     std::cout << "Orders processed: " << numOrders << "\n";
     std::cout << "Trades executed: " << book.getTrades().size() << "\n";
     std::cout << "Elapsed time: " << elapsed.count() << " sec\n";
     std::cout << "Throughput: " << tradesPerSec << " trades/sec\n";
 
     return 0;
 }
 