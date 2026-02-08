#ifndef COMMAND_H
#define COMMAND_H

#include "matching_engine/enum.h"
#include <string>

class Command {
public:
    Command(uint64_t clientId, uint64_t orderId, std::string symbol);
    virtual ~Command() = default;
    virtual CommandType getType() const = 0;
    uint64_t getClientId() const;
    uint64_t getOrderId() const;
    std::string getSymbol() const;

private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol;
};



#endif // COMMAND_H