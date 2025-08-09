/**
 * @file order_book.h
 * @brief Declares the OrderBook class for managing a high-performance limit order book matching engine.
 *
 * The OrderBook supports adding, modifying, canceling orders, executing trades with
 * price-time priority, and exporting snapshots/trade history. It is optimized for
 * sequential matching performance while keeping the interface simple.
 *
 * @author
 *   Nick Ingargiola
 * @date
 *   2025-08-08
 */

 #ifndef ORDER_BOOK_H
 #define ORDER_BOOK_H
 
 #include "order.h"
 #include "trade.h"
 #include <map>
 #include <queue>
 #include <vector>
 #include <unordered_map>
 #include <string>
 #include <list>
 
 /**
  * @class OrderBook
  * @brief Maintains bid/ask lists, matches trades, and tracks executed orders.
  *
  * Orders are stored in FIFO order per price level using std::list.
  * The engine matches based on price-time priority:
  *  - Highest bid matches lowest ask
  *  - If prices cross, trades are executed
  *  - Partial fills are supported
  */
 class OrderBook
 {
 public:
     /**
      * @brief Add an order to the book and attempt to match it immediately.
      * @param order The order to insert.
      */
     void addOrder(const Order &order);
 
     /**
      * @brief Modify an existing order's quantity and price.
      * @param id Order ID to modify.
      * @param newQty New quantity.
      * @param newPrice New price.
      * @param newTimestamp New timestamp for time-priority updates.
      * @return true if the order was found and modified, false otherwise.
      */
     bool modifyOrder(int id, int newQty, double newPrice, long newTimestamp);
 
     /**
      * @brief Cancel an existing order by ID.
      * @param id Order ID to cancel.
      * @return true if canceled, false if not found.
      */
     bool cancelOrder(int id);
 
     /**
      * @brief Attempt to match the top of the book (highest bid vs lowest ask).
      *
      * Called internally after adds/modifications, but can also be invoked manually.
      */
     void matchOrders();
 
     /**
      * @brief Print the current order book to stdout.
      */
     void printBook() const;
 
     /**
      * @brief Print the trade history to stdout.
      */
     void printTrades() const;
 
     /**
      * @brief Export trades to CSV.
      * @param baseName Output filename prefix (timestamp and sequence appended automatically).
      */
     void exportTradesCSV(const std::string &baseName = "trades") const;
 
     /**
      * @brief Export current order book snapshot to CSV.
      * @param baseName Output filename prefix.
      */
     void exportBookCSV(const std::string &baseName = "book") const;
 
     /**
      * @brief Set the directory for CSV exports.
      * @param dir Target directory.
      */
     void setExportDir(const std::string &dir);
 
     /**
      * @brief Enable or disable automatic export after each trade.
      * @param on True to enable, false to disable.
      */
     void setAutoExport(bool on) { autoExport = on; };
 
     /**
      * @brief Accessor for executed trades.
      * @return Const reference to the vector of trades.
      */
     const std::vector<Trade> &getTrades() const { return trades; }
 
 private:
     /**
      * @brief Internal mapping from order ID to its list iterator for O(1) lookup.
      */
     struct IdInfo
     {
         OrderType type;                  ///< Order side (BUY/SELL)
         std::list<Order>::iterator it;   ///< Iterator to order in the list
     };
 
     std::list<Order> bids;               ///< Buy orders (sorted by price desc, FIFO per price)
     std::list<Order> asks;               ///< Sell orders (sorted by price asc, FIFO per price)
 
     std::unordered_map<int, IdInfo> orderIndex; ///< Fast lookup for cancel/modify
 
     int totalVolumeTraded = 0;           ///< Cumulative traded quantity
     std::vector<Trade> trades;           ///< Executed trades
 
     /**
      * @brief Execute a trade between two orders.
      * @param buy Reference to the buy order.
      * @param sell Reference to the sell order.
      */
     void executeTrade(Order &buy, Order &sell);
 
     bool autoExport = true;              ///< If true, export after each trade
     std::string exportDir = "exports";   ///< Directory for export files
 };
 
 #endif // ORDER_BOOK_H
 