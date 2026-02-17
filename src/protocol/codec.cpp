#include "protocol/codec.h"
#include "matching_engine/command/limit_command.h"
#include "matching_engine/command/market_command.h" 
#include "matching_engine/command/cancel_command.h"
#include "protocol/helpers.h"

Codec::Codec() {}

std::vector<std::uint8_t> Codec::encodeCommand(const Command& cmd) {
    // encode command attributes
    std::vector<std::uint8_t> payloadBytes;
    uint64_t clientId = cmd.getClientId();
    uint64_t orderId = cmd.getOrderId();
    std::string symbol = cmd.getSymbol();
    uint8_t len = symbol.size();
    CommandType type = cmd.getType();
    uint8_t t = static_cast<uint8_t>(type);

    write_u8(payloadBytes, t);
    write_u64_be(payloadBytes, clientId);
    write_u64_be(payloadBytes, orderId);
    write_u8(payloadBytes, len);
    payloadBytes.insert(payloadBytes.end(), symbol.begin(), symbol.end());

    switch (type) { //encode specific command attributes
    case CommandType::Limit: {
        const auto& limitCmd = static_cast<const LimitCommand&>(cmd);
        int64_t price = limitCmd.getPrice();
        int64_t qty = limitCmd.getQty();
        uint8_t side = static_cast<uint8_t>(limitCmd.getSide());

        write_i64_be(payloadBytes, price);
        write_i64_be(payloadBytes, qty);
        write_u8(payloadBytes, side);
        break;
    }
    case CommandType::Market: {
        const auto& marketCmd = static_cast<const MarketCommand&>(cmd);
        int64_t qty = marketCmd.getQty();
        uint8_t side = static_cast<uint8_t>(marketCmd.getSide());
    
        write_i64_be(payloadBytes, qty);
        write_u8(payloadBytes, side);
        break;
    }
    case CommandType::Cancel: {
        break;
    }
    default:{
        break;
    }
    }
    return payloadBytes;
}