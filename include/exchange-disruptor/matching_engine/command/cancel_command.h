#ifndef CANCEL_COMMAND_H
#define CANCEL_COMMAND_H

#include "command.h"

class CancelCommand : public Command {
public:
    CancelCommand(uint64_t clientId, uint64_t orderId, std::string symbol);
    CommandType getType() const override;
};

#endif // CANCEL_COMMAND_H