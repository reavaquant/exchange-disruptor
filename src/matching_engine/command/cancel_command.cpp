#include "matching_engine/command/cancel_command.h"

CancelCommand::CancelCommand(uint64_t clientId, uint64_t orderId, std::string symbol) : Command(clientId, orderId, symbol) {}

const CommandType& CancelCommand::getType() const {
    static const CommandType kType = CommandType::Cancel;
    return kType;
}
