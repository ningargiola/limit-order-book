/**
 * @file test_order_book.cpp
 * @brief GoogleTest suite for validating the core functionality of the Limit Order Book.
 * @author Nick Ingargiola
 *
 * Tests include:
 *  - Basic matching and partial fills
 *  - FIFO matching at identical price levels
 *  - Cancel/modify operations
 *  - Edge cases (large IDs, zero qty, high precision prices, huge prices)
 *  - Performance benchmarking
 *  - Integration with live feed
 *  - Bulk cancelation scenarios
 */

 #include <gtest/gtest.h>
 #include "order_book.h"
 #include <random>
 #include <sstream>
 
 /**
  * @brief Test fixture providing a fresh OrderBook for each test.
  */
 struct OBFixture : ::testing::Test {
     OrderBook book;
     long ts = 1;     ///< Simulated timestamp incremented per order
     int nextId = 1;  ///< Incrementing order ID counter
 
     void SetUp() override {
         book.setAutoExport(false);      // Disable CSV exports for test speed
         book.setExportDir("exports");   // Harmless; ensures directory exists
     }
 
     /**
      * @brief Helper to insert a new order into the book.
      * @param t   Order side (BUY/SELL)
      * @param px  Price
      * @param qty Quantity
      * @return The assigned order ID
      */
     int add(OrderType t, double px, int qty) {
         int id = nextId++;
         book.addOrder(Order(id, t, px, qty, ts++));
         return id;
     }
 };
 
 // ------------------ CORE MATCHING TESTS ------------------
 
 /** @test Basic cross where the sell partially fills the buy. */
 TEST_F(OBFixture, SimpleCross_PartialFill) {
     int b1 = add(OrderType::BUY, 100.0, 10);
     int s1 = add(OrderType::SELL, 99.0, 5);
 
     const auto &tr = book.getTrades();
     ASSERT_EQ(tr.size(), 1u);
     EXPECT_EQ(tr[0].buyId, b1);
     EXPECT_EQ(tr[0].sellId, s1);
     EXPECT_DOUBLE_EQ(tr[0].price, 99.0);
     EXPECT_EQ(tr[0].quantity, 5);
 }
 
 /** @test Two trades that completely consume the bid, leaving book empty. */
 TEST_F(OBFixture, TwoTradesThenEmpty) {
     add(OrderType::BUY, 100.0, 7);
     add(OrderType::SELL, 100.0, 3);
     add(OrderType::SELL, 100.0, 4);
 
     const auto &tr = book.getTrades();
     ASSERT_EQ(tr.size(), 2u);
     EXPECT_EQ(tr[0].quantity, 3);
     EXPECT_EQ(tr[1].quantity, 4);
 }
 
 /** @test FIFO priority is respected for orders at same price. */
 TEST_F(OBFixture, FifoAtSamePrice) {
     int s2 = add(OrderType::SELL, 101.0, 4);
     int s3 = add(OrderType::SELL, 101.0, 5);
     int b1 = add(OrderType::BUY, 101.0, 6);
 
     const auto &tr = book.getTrades();
     ASSERT_EQ(tr.size(), 2u);
     EXPECT_EQ(tr[0].sellId, s2); // Earliest sell first
     EXPECT_EQ(tr[0].quantity, 4);
     EXPECT_EQ(tr[1].sellId, s3);
     EXPECT_EQ(tr[1].quantity, 2);
 }
 
 /** @test No trade occurs if highest bid < lowest ask. */
 TEST_F(OBFixture, NoCrossWhenBidBelowAsk) {
     add(OrderType::BUY, 99.0, 5);
     add(OrderType::SELL, 100.0, 5);
     EXPECT_TRUE(book.getTrades().empty());
 }
 
 // ------------------ CANCEL / MODIFY TESTS ------------------
 
 /** @test Cancelation works and handles non-existent IDs gracefully. */
 TEST_F(OBFixture, CancelWorks) {
     int b1 = add(OrderType::BUY, 100.0, 5);
     int b2 = add(OrderType::BUY, 100.0, 6);
     EXPECT_TRUE(book.cancelOrder(b2));
     EXPECT_FALSE(book.cancelOrder(999)); // Fake ID
     EXPECT_TRUE(book.cancelOrder(b1));
 }
 
 /** @test Modify changes both price and quantity, potentially triggering a trade. */
 TEST_F(OBFixture, ModifyChangesPriceAndQty) {
     int s1 = add(OrderType::SELL, 101.0, 10);
     int b1 = add(OrderType::BUY, 100.0, 6);
     EXPECT_TRUE(book.modifyOrder(s1, 8, 100.0, ts++)); // Now crosses
 
     const auto &tr = book.getTrades();
     ASSERT_FALSE(tr.empty());
     EXPECT_EQ(tr.back().price, 100.0);
     EXPECT_EQ(tr.back().quantity, 6);
 }
 
 /** @test Handles very large order IDs without overflow issues. */
 TEST_F(OBFixture, LargeOrderId) {
     int bigId = 1'000'000'000;
     book.addOrder(Order(bigId, OrderType::BUY, 105.0, 5, 1));
     ASSERT_EQ(book.getTrades().size(), 0);
     ASSERT_TRUE(book.cancelOrder(bigId));
 }
 
 /** @test Orders with zero quantity are ignored and never matched. */
 TEST_F(OBFixture, ZeroQuantityIgnored) {
     book.addOrder(Order(1, OrderType::BUY, 100.0, 0, 1));
     book.addOrder(Order(2, OrderType::SELL, 99.0, 5, 2));
     ASSERT_EQ(book.getTrades().size(), 0);
 }
 
 /** @test High-precision floating point prices are accepted. */
 TEST_F(OBFixture, HighPrecisionPrice) {
     double price = 100.123456789;
     book.addOrder(Order(1, OrderType::BUY, price, 5, 1));
     ASSERT_EQ(book.getTrades().size(), 0);
 }
 
 // ------------------ PERFORMANCE / INTEGRATION ------------------
 
 /**
  * @test Benchmark throughput on synthetic random orders.
  * Fails if throughput falls below `minTradesPerSec`.
  */
 TEST_F(OBFixture, ThroughputBenchmark) {
     const int numOrders = 500000;
     const double minTradesPerSec = 400000;
 
     std::mt19937 rng(42);
     std::uniform_real_distribution<double> priceDist(90.0, 110.0);
     std::uniform_int_distribution<int> qtyDist(1, 5);
     std::uniform_int_distribution<int> sideDist(0, 1);
 
     auto start = std::chrono::high_resolution_clock::now();
     for (int i = 0; i < numOrders; i++) {
         OrderType type = sideDist(rng) ? OrderType::BUY : OrderType::SELL;
         book.addOrder(Order(nextId++, type, priceDist(rng), qtyDist(rng), ts++));
     }
     auto end = std::chrono::high_resolution_clock::now();
 
     double tradesPerSec = book.getTrades().size() /
                           std::chrono::duration<double>(end - start).count();
     std::cout << "Benchmark trades/sec: " << tradesPerSec << "\n";
     ASSERT_GT(tradesPerSec, minTradesPerSec);
 }
 
 /**
  * @test Integration with live feed via Python WebSocket streamer.
  * Verifies that at least one trade occurs in a 3-second window.
  */
 TEST_F(OBFixture, RealFeedIntegration) {
     FILE* pipe = popen("python3 ws_feeder.py", "r");
     ASSERT_NE(pipe, nullptr);
 
     char buffer[256];
     auto start = std::chrono::steady_clock::now();
     while (std::chrono::steady_clock::now() - start < std::chrono::seconds(3)) {
         if (fgets(buffer, sizeof(buffer), pipe)) {
             std::istringstream iss(buffer);
             std::string sideStr;
             double price; int qty;
             if (iss >> sideStr >> price >> qty) {
                 OrderType type = (sideStr == "BUY") ? OrderType::BUY : OrderType::SELL;
                 book.addOrder(Order(nextId++, type, price, qty, ts++));
             }
         }
     }
     pclose(pipe);
 
     std::cout << "Trades executed from feed: " << book.getTrades().size() << "\n";
     ASSERT_GT(book.getTrades().size(), 0);
 }
 
 // ------------------ BULK / EDGE SCENARIOS ------------------
 
 /** @test Mass cancelation of 100 orders succeeds. */
 TEST_F(OBFixture, MassCancel) {
     for (int i = 0; i < 100; ++i) add(OrderType::BUY, 100.0, 5);
     for (int i = 1; i <= 100; ++i) EXPECT_TRUE(book.cancelOrder(i));
     EXPECT_EQ(book.getTrades().size(), 0);
 }
 
 /** @test Cancel after fill should fail (already removed from book). */
 TEST_F(OBFixture, CancelAfterFill) {
     int b = add(OrderType::BUY, 101.0, 5);
     add(OrderType::SELL, 100.0, 5); // Instantly fills
     EXPECT_FALSE(book.cancelOrder(b));
 }
 
 /** @test Orders with extremely large prices are accepted. */
 TEST_F(OBFixture, HugePriceValues) {
     int id = add(OrderType::BUY, 1e9, 5);
     EXPECT_TRUE(id > 0);
     EXPECT_EQ(book.getTrades().size(), 0);
 }
 