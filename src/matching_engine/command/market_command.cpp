#include "matching_engine/command/market_command.h"

MarketCommand::MarketCommand(uint64_t clientId, uint64_t orderId, std::string symbol, Side side, double qty) : Command(clientId, orderId, symbol), _qty(qty), _side(side) {
    if (qty <= 0) {
        throw std::invalid_argument("Quantity must be greater than 0");
    }
    if (side != Side::Buy && side != Side::Sell) {
        throw std::invalid_argument("Invalid side");
    }
}

const CommandType& MarketCommand::getType() const {
    static const CommandType kType = CommandType::Market;
    return kType;
}

const double& MarketCommand::getQty() const {
    return _qty;
}

const Side& MarketCommand::getSide() const {
    return _side;
}
