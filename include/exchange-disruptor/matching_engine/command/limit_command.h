#ifndef LIMIT_COMMAND_H
#define LIMIT_COMMAND_H

#include "matching_engine/command/command.h"

class LimitCommand : public Command {
public:
    LimitCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, double qty);
    const CommandType& getType() const override;
    const int64_t& getPrice() const;
    const double& getQty() const;
    const Side& getSide() const;

private:
    int64_t _price;
    double _qty;
    Side _side;
};

#endif // LIMIT_COMMAND_H