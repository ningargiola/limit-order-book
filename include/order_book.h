#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "order.h"
#include <map>
#include <queue>

class OrderBook {
    public:
        void addOrder(const Order& order);
        bool modifyOrder(int id, int newQty, double newPrice, long newTimestamp);
        bool cancelOrder(int id);
        void matchOrders();
        void printBook() const;

    
    private:
        std::map<double, std::queue<Order>, std::greater<double>> bids;
        std::map<double, std::queue<Order>> asks;

        int totalVolumeTraded = 0;

        void executeTrade(Order& buy, Order& sell);
};

#endif