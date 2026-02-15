#include "matching_engine/command/limit_command.h"
#include "matching_engine/reject_exception.h"

LimitCommand::LimitCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, int64_t qty) : Command(clientId, orderId, symbol), _price(price), _qty(qty), _side(side) {
    if (price <= 0) {
        throw RejectException(RejectReason::InvalidPrice);
    }
    if (qty <= 0) {
        throw RejectException(RejectReason::InvalidQuantity);
    }
    if (side != Side::Buy && side != Side::Sell) {
        throw RejectException(RejectReason::InternalError);
    }
}

CommandType LimitCommand::getType() const { return CommandType::Limit; }

int64_t LimitCommand::getPrice() const { return _price; }

int64_t LimitCommand::getQty() const { return _qty; }

Side LimitCommand::getSide() const { return _side; }
