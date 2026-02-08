#include "matching_engine/command/cancel_command.h"

CancelCommand::CancelCommand(uint64_t clientId, uint64_t orderId, std::string symbol) : Command(clientId, orderId, symbol) {}

CommandType CancelCommand::getType() const { return CommandType::Cancel; }