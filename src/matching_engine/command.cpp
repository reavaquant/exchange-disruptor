#include "command.h"


Command::Command(int64_t clientId, int64_t orderId, std::string symbol) : _clientId(clientId), _orderId(orderId), _symbol(symbol) {}

Command::~Command() = default;

int64_t Command::getClientId() const { return _clientId; }
int64_t Command::getOrderId() const { return _orderId; }
std::string Command::getSymbol() const { return _symbol; }
