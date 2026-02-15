#ifndef MARKET_COMMAND_H
#define MARKET_COMMAND_H

#include "matching_engine/command/command.h"

class MarketCommand : public Command {
public:
    MarketCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t qty);
    CommandType getType() const override;
    int64_t getQty() const;
    Side getSide() const;
private:
    int64_t _qty;
    Side _side;
};

#endif // MARKET_COMMAND_H
