#include "matching_engine/command/command.h"


Command::Command(uint64_t clientId, uint64_t orderId, std::string symbol) : _clientId(clientId), _orderId(orderId), _symbol(symbol) {}

uint64_t Command::getClientId() const { return _clientId; }
uint64_t Command::getOrderId() const { return _orderId; }
std::string Command::getSymbol() const { return _symbol; }
