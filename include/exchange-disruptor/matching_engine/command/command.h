#ifndef COMMAND_H
#define COMMAND_H

#include "matching_engine/enum.h"
#include <string>

class Command {
public:
    Command(uint64_t clientId, uint64_t orderId, std::string symbol);
    virtual ~Command() = default;
    virtual const CommandType& getType() const = 0;
    const uint64_t& getClientId() const;
    const uint64_t& getOrderId() const;
    const std::string& getSymbol() const;

private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol; 
};

#endif // COMMAND_H