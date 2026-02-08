#include "matching_engine/command/limit_command.h"

LimitCommand::LimitCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, int64_t price, double qty) : Command(clientId, orderId, symbol), _price(price), _qty(qty), _side(side) {
    if (price <= 0) {
        throw std::invalid_argument("Price must be greater than 0");
    }
    if (qty <= 0) {
        throw std::invalid_argument("Quantity must be greater than 0");
    }
    if (side != Side::Buy && side != Side::Sell) {
        throw std::invalid_argument("Invalid side");
    }
}

const CommandType& LimitCommand::getType() const {
    static const CommandType kType = CommandType::Limit;
    return kType;
}

const int64_t& LimitCommand::getPrice() const {
    return _price;
}

const double& LimitCommand::getQty() const {
    return _qty;
}

const Side& LimitCommand::getSide() const {
    return _side;
}
