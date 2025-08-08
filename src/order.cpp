#include "order.h"

Order::Order(int id, OrderType type, double price, int quantity, long timestamp)
    : id(id), type(type), price(price), quantity(quantity), timestamp(timestamp) {}