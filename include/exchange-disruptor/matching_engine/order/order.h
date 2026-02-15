#ifndef ORDER_H
#define ORDER_H

#include "matching_engine/enum.h"
#include <string>

class Order {
public:
    Order(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, int64_t qtyRemaining);
    uint64_t getClientId() const;
    uint64_t getOrderId() const;
    const std::string& getSymbol() const &;
    const std::string& getSymbol() const && = delete;
    Side getSide() const;
    int64_t getPrice() const;
    int64_t getQtyRemaining() const;
    int64_t qtyDecrease(int64_t execQty);
private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol;
    Side _side;
    int64_t _price;
    int64_t _qtyRemaining;
};  


#endif // ORDER_H
