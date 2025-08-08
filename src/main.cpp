#include "order_book.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

int main()
{
    OrderBook book;

    long timestamp = 1;
    int nextId = 1;

    std::vector<std::string> commands = {
        "BUY 100 10",
        "SELL 99 5",
        "SELL 99 5",
        "SELL 102 10",
        "BUY 102 5",
        "MODIFY 4 8 101",
        "CANCEL 5",
        "PRINT"
    };

    for (const auto &input : commands)
    {
        std::cout << ">" << input << std::endl;
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "BUY" || command == "SELL")
        {
            double price;
            int qty;

            if (iss >> price >> qty)
            {
                OrderType type = (command == "BUY") ? OrderType::BUY : OrderType::SELL;
                book.addOrder(Order(nextId++, type, price, qty, timestamp++));
            }
        }
        else if (command == "CANCEL")
        {
            int id;
            if (iss >> id)
            {
                bool success = book.cancelOrder(id);
                std::cout << (success ? "Order cancelled.\n" : "Order not found.\n");
            }
        }
        else if (command == "MODIFY")
        {
            int id, qty;
            double price;
            if (iss >> id >> qty >> price)
            {
                bool success = book.modifyOrder(id, qty, price, timestamp++);
                std::cout << (success ? "Order modified.\n" : "Order not found.\n");
            }
        }
        else if (command == "PRINT")
        {
            book.printBook();
        }
        else if (command == "EXIT")
        {
            break;
        }
    }
    return 0;
}
