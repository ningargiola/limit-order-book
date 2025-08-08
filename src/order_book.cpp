#include "order_book.h"
#include <iostream>
#include <algorithm>
#include <optional>

void OrderBook::addOrder(const Order &order)
{
    if (order.type == OrderType::BUY)
    {
        bids[order.price].push(order);
    }
    else
    {
        asks[order.price].push(order);
    }
    matchOrders();
}

bool OrderBook::modifyOrder(int id, int newQty, double newPrice, long newTimestamp)
{
    auto findAndCopy = [&](auto &side, OrderType type) -> std::optional<Order>
    {
        for (const auto &[price, q] : side)
        {
            std::queue<Order> temp = q;
            while (!temp.empty())
            {
                Order o = temp.front();
                temp.pop();
                std::cout << "Checking order ID: " << o.id << std::endl; // 🐛 DEBUG
                if (o.id == id)
                    return Order{id, type, newPrice, newQty, newTimestamp};
            }
        }
        return std::nullopt;
    };

    auto modified = findAndCopy(bids, OrderType::BUY);
    if (!modified)
    {
        modified = findAndCopy(asks, OrderType::SELL);
        if (!modified)
            return false;
    }

    if (!cancelOrder(id))
        return false;

    addOrder(*modified);
    return true;
}

bool OrderBook::cancelOrder(int id)
{
    auto cancelFromSide = [&](auto &side) -> bool
    {
        for (auto it = side.begin(); it != side.end(); ++it)
        {
            std::queue<Order> &q = it->second;
            std::queue<Order> newQ;
            bool found = false;

            while (!q.empty())
            {
                Order order = q.front();
                q.pop();

                if (order.id == id)
                {
                    found = true;
                    continue;
                }
                newQ.push(order);
            }

            if (found)
            {
                if (newQ.empty())
                    side.erase(it);
                else
                    it->second = std::move(newQ);
                return true;
            }
        }
        return false;
    };

    return cancelFromSide(bids) || cancelFromSide(asks);
}

void OrderBook::matchOrders()
{
    while (!bids.empty() && !asks.empty())
    {
        auto bestBid = bids.begin();
        auto bestAsk = asks.begin();

        if (bestBid->first >= bestAsk->first)
        {
            Order &buy = bestBid->second.front();
            Order &sell = bestAsk->second.front();

            executeTrade(buy, sell);

            if (buy.quantity == 0)
                bestBid->second.pop();
            if (sell.quantity == 0)
                bestAsk->second.pop();

            if (bestBid->second.empty())
                bids.erase(bestBid);
            if (bestAsk->second.empty())
                asks.erase(bestAsk);
        }
        else
        {
            break;
        }
    }
}

void OrderBook::executeTrade(Order &buy, Order &sell)
{
    int qty = std::min(buy.quantity, sell.quantity);
    std::cout << "TRADE: " << qty << " @ $" << sell.price << std::endl;

    buy.quantity -= qty;
    sell.quantity -= qty;

    totalVolumeTraded += qty;
}

void OrderBook::printBook() const
{
    std::cout << "\nOrder Book:\n";
    std::cout << "BIDS:\n";
    for (const auto &[price, orders] : bids)
    {
        if (orders.empty()) continue;
        std::queue<Order> temp = orders;
        std::cout << " $" << price << " x " << orders.size() << " orders: ";
        while (!temp.empty()) {
            const Order& o = temp.front();
            std::cout << "[ID " << o.id << ", qty" << o.quantity << "]";
            temp.pop();
        }
        std::cout << "\n";
    }

    std::cout << "ASKS:\n";
    for (const auto &[price, orders] : asks)
    {
        if (orders.empty()) continue;
        std::queue<Order> temp = orders;
        std::cout << " $" << price << " x " << orders.size() << " orders: ";
        while (!temp.empty()) {
            const Order& o = temp.front();
            std::cout << "[ID " << o.id << ", qty" << o.quantity << "]";
            temp.pop();
        }
        std::cout << "\n";
    }

    std::cout << "Total Volume Traded: " << totalVolumeTraded << " units\n";
}