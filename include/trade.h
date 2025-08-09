/**
 * @file trade.h
 * @brief Defines the Trade struct representing an executed match between a buy and sell order.
 *
 * @author Nick Ingargiola
 * @date 2025-08-08
 */

 #ifndef TRADE_H
 #define TRADE_H
 
 #include <cstdint>
 
 /**
  * @struct Trade
  * @brief Represents a single executed trade in the order book.
  *
  * A trade occurs when a buy order and a sell order match according to the
  * order book's price-time priority rules. The trade records both order IDs,
  * the execution price, the quantity traded, and the timestamp.
  */
 struct Trade
 {
     int buyId;       ///< The ID of the buy order.
     int sellId;      ///< The ID of the sell order.
     double price;    ///< The execution price of the trade.
     int quantity;    ///< The quantity of units traded.
     long timestamp;  ///< Logical or real timestamp of when the trade occurred.
 
     /**
      * @brief Constructs a Trade instance.
      * @param buyId The ID of the buy order.
      * @param sellId The ID of the sell order.
      * @param price The execution price.
      * @param quantity The quantity traded.
      * @param timestamp The time the trade occurred.
      */
     Trade(int buyId, int sellId, double price, int quantity, long timestamp)
         : buyId(buyId), sellId(sellId), price(price), quantity(quantity), timestamp(timestamp) {}
 };
 
 #endif // TRADE_H
 