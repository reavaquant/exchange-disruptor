#ifndef ORDER_H
#define ORDER_H

#include "matching_engine/enum.h"
#include <string>

class Order {
public:
    Order(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, double qtyRemaining);
    uint64_t getClientId() const;
    uint64_t getOrderId() const;
    const std::string& getSymbol() const &;
    const std::string& getSymbol() const && = delete;
    Side getSide() const;
    int64_t getPrice() const;
    double getQtyRemaining() const;
    bool isFilled() const;
    double qtyDecrease(double execQty);
private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol;
    Side _side;
    int64_t _price;
    double _qtyRemaining;
    bool _isFilled{false};
};  


#endif // ORDER_H
