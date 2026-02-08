#ifndef LIMIT_COMMAND_H
#define LIMIT_COMMAND_H

#include "command.h"

class LimitCommand : public Command {
public:
    LimitCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, double qty) : Command(clientId, orderId, symbol), _price(price), _qty(qty), _side(side) {}
    CommandType getType() const override;
    int64_t getPrice() const;
    double getQty() const;
    Side getSide() const;

private:
    int64_t _price;
    double _qty;
    Side _side;
};

#endif // LIMIT_COMMAND_H