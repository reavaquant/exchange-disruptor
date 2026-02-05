#ifndef ORDER_H
#define ORDER_H

#include "enum.h"
#include <string>

class Order {
public:
    Order(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, double price, double qtyRemaining) : _clientId(clientId), _orderId(orderId), _symbol(symbol), _side(side), _price(price), _qtyRemaining(qtyRemaining) {}
    uint64_t getClientId() const;
    uint64_t getOrderId() const;
    std::string getSymbol() const;
    Side getSide() const;
    double getPrice() const;
    double getQtyRemaining() const;
    bool isFilled() const;
    double qtyDecrease(double execQty);
private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol;
    Side _side;
    double _price;
    double _qtyRemaining;
    bool _isFilled{false};
};  


#endif // ORDER_H
