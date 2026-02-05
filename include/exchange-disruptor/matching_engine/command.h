#ifndef COMMAND_H
#define COMMAND_H

#include "enum.h"
#include <string>

class Command {
public:
    Command(uint64_t clientId, uint64_t orderId, std::string symbol) : _clientId(clientId), _orderId(orderId), _symbol(symbol) {}
    virtual ~Command() = default;
    virtual CommandType getType() const = 0;
    uint64_t getClientId() const { return _clientId; }
    uint64_t getOrderId() const { return _orderId; }
    std::string getSymbol() const { return _symbol; }

private:
    uint64_t _clientId;
    uint64_t _orderId;
    std::string _symbol;
};



#endif // COMMAND_H