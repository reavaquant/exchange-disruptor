#ifndef MARKET_COMMAND_H
#define MARKET_COMMAND_H

#include "command.h"

class MarketCommand : public Command {
public:
    MarketCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, double qty) : Command(clientId, orderId, symbol), _qty(qty), _side(side) {}
    CommandType getType() const override;
    double getQty() const;
    Side getSide() const;
private:
    double _qty;
    Side _side;
};

#endif // MARKET_COMMAND_H