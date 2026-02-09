#ifndef LIMIT_COMMAND_H
#define LIMIT_COMMAND_H

#include "matching_engine/command/command.h"

class LimitCommand : public Command {
public:
    LimitCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, int64_t qty);
    CommandType getType() const override;
    int64_t getPrice() const;
    int64_t getQty() const;
    Side getSide() const;

private:
    int64_t _price;
    int64_t _qty;
    Side _side;
};

#endif // LIMIT_COMMAND_H
