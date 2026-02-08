#include "matching_engine/command/command.h"


Command::Command(uint64_t clientId, uint64_t orderId, std::string symbol) : _clientId(clientId), _orderId(orderId), _symbol(symbol) {}

const uint64_t& Command::getClientId() const { return _clientId; }
const uint64_t& Command::getOrderId() const { return _orderId; }
const std::string& Command::getSymbol() const { return _symbol; }