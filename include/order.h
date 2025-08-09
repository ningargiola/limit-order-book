/**
 * @file order.h
 * @brief Defines the Order struct and OrderType enum for use in the limit order book.
 *
 * Orders represent individual buy or sell instructions with an ID, side,
 * price, quantity, and timestamp. They are the fundamental building blocks
 * processed by the OrderBook.
 *
 * @author
 *   Nick Ingargiola
 * @date
 *   2025-08-08
 */

 #ifndef ORDER_H
 #define ORDER_H
 
 #include <string>
 
 /**
  * @enum OrderType
  * @brief Represents the side of an order in the order book.
  */
 enum class OrderType
 {
     BUY,   ///< Buy order (bid)
     SELL   ///< Sell order (ask)
 };
 
 /**
  * @struct Order
  * @brief Represents a single limit order.
  *
  * Each order has:
  *  - A unique ID (for tracking and modification)
  *  - A side (BUY or SELL)
  *  - A limit price
  *  - A quantity to trade
  *  - A timestamp (used for price-time priority)
  */
 struct Order
 {
     int id;            ///< Unique order identifier
     OrderType type;    ///< BUY or SELL
     double price;      ///< Limit price
     int quantity;      ///< Quantity remaining in the order
     long timestamp;    ///< Timestamp for FIFO priority at the same price
 
     /**
      * @brief Construct a new Order.
      * @param id Unique order ID.
      * @param type Side of the order (BUY/SELL).
      * @param price Limit price.
      * @param quantity Order quantity.
      * @param timestamp Time the order was placed.
      */
     Order(int id, OrderType type, double price, int quantity, long timestamp);
 };
 
 #endif // ORDER_H
 