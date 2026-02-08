#ifndef MARKET_COMMAND_H
#define MARKET_COMMAND_H

#include "matching_engine/command/command.h"

class MarketCommand : public Command {
public:
    MarketCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, double qty);
    const CommandType& getType() const override;
    const double& getQty() const;
    const Side& getSide() const;
private:
    double _qty;
    Side _side;
};

#endif // MARKET_COMMAND_H