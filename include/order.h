#ifndef ORDER_H
#define ORDER_H

#include <string>

enum class OrderType { BUY, SELL };

struct Order {
    int id;
    OrderType type;
    double price;
    int quantity;
    long timestamp;

    Order(int id, OrderType type, double price, int quantity, long timestamp);
};

#endif