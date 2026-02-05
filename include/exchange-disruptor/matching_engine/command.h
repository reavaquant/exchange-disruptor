#ifndef COMMAND_H
#define COMMAND_H

#include "enum.h"
#include <string>

class Command {
public:
    Command(int64_t clientId, int64_t orderId, std::string symbol) : _clientId(clientId), _orderId(orderId), _symbol(symbol) {}
    virtual ~Command() = default;
    virtual CommandType getType() const = 0;
    int64_t getClientId() const { return _clientId; }
    int64_t getOrderId() const { return _orderId; }
    std::string getSymbol() const { return _symbol; }

private:
    int64_t _clientId;
    int64_t _orderId;
    std::string _symbol;
};



#endif // COMMAND_H