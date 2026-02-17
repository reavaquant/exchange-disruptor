#include "protocol/codec.h"
#include "matching_engine/command/limit_command.h"
#include "matching_engine/command/market_command.h" 
#include "matching_engine/command/cancel_command.h"
#include "matching_engine/event/fill_event.h"
#include "matching_engine/event/reject_event.h"
#include "matching_engine/event/ack_event.h"
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

std::vector<std::uint8_t> Codec::encodeEvent(const Event& event) {
    std::vector<std::uint8_t> payloadBytes;
    EventType type = event.getType();
    uint8_t t = static_cast<uint8_t>(type);
    uint64_t clientId = event.getClientId();
    uint64_t orderId = event.getOrderId();

    write_u8(payloadBytes, t);
    write_u64_be(payloadBytes, clientId);
    write_u64_be(payloadBytes, orderId);

    switch (type)
    {
    case EventType::Fill: {
        const auto& fill = static_cast<const FillEvent&>(event);
        uint64_t matchId = fill.getMatchId();
        int64_t price = fill.getPrice();
        int64_t qty = fill.getQty();

        write_u64_be(payloadBytes, matchId);
        write_i64_be(payloadBytes, price);
        write_i64_be(payloadBytes, qty);
        break;
    }
    case EventType::Reject: {
        const auto& reject = static_cast<const RejectEvent&>(event);
        uint8_t reason = static_cast<uint8_t>(reject.getReason());

        write_u8(payloadBytes, reason);
        break;
    }
    case EventType::Ack: {
        break;
    }
    default:
        break;
    }
    return payloadBytes;
}


std::unique_ptr<Command> Codec::decodeCommand(const std::vector<std::uint8_t>& payloadBytes) {
    std::size_t off = 0;

    auto need = [&](std::size_t n) { return off + n <= payloadBytes.size(); }; //lambda function for payload size check

    if (!need(1)) return nullptr;
    auto type = static_cast<CommandType>(payloadBytes[off++]);
    if (type != CommandType::Limit && type != CommandType::Market && type != CommandType::Cancel) {
        return nullptr;
    }

    uint64_t clientId = 0, orderId = 0;
    if (!read_u64_be(payloadBytes, off, clientId)) return nullptr;
    if (!read_u64_be(payloadBytes, off, orderId))  return nullptr;

    if (!need(1)) return nullptr;
    uint8_t len = payloadBytes[off++];

    if (!need(len)) return nullptr;
    std::string symbol(payloadBytes.begin() + off, payloadBytes.begin() + off + len);
    off += len;

    switch (type) {
        case CommandType::Limit: {
            int64_t price = 0, qty = 0;
            if (!read_i64_be(payloadBytes, off, price)) return nullptr;
            if (!read_i64_be(payloadBytes, off, qty)) return nullptr;

            if (!need(1)) return nullptr;
            auto side = static_cast<Side>(payloadBytes[off++]);
            if (side != Side::Buy && side != Side::Sell) return nullptr;

            return std::make_unique<LimitCommand>(clientId, orderId, symbol, side, price, qty);
        }
        case CommandType::Market: {
            int64_t qty = 0;
            if (!read_i64_be(payloadBytes, off, qty)) return nullptr;

            if (!need(1)) return nullptr;
            auto side = static_cast<Side>(payloadBytes[off++]);
            if (side != Side::Buy && side != Side::Sell) return nullptr;

            return std::make_unique<MarketCommand>(clientId, orderId, symbol, side, qty);
        }
        case CommandType::Cancel: {
            return std::make_unique<CancelCommand>(clientId, orderId, symbol);
        }
        default:
            return nullptr;
    }
}

std::unique_ptr<Event> Codec::decodeEvent(const std::vector<std::uint8_t>& payloadBytes) {
    std::size_t off = 0;

    auto need = [&](std::size_t n) { return off + n <= payloadBytes.size(); }; //lambda function for payload size check

    if (!need(1)) return nullptr;
    auto type = static_cast<EventType>(payloadBytes[off++]);
    if (type != EventType::Fill && type != EventType::Reject && type != EventType::Ack) {
        return nullptr;
    }

    uint64_t clientId = 0, orderId = 0;
    if (!read_u64_be(payloadBytes, off, clientId)) return nullptr;
    if (!read_u64_be(payloadBytes, off, orderId))  return nullptr;

    switch (type) {
        case EventType::Fill: {
            uint64_t matchId = 0;
            int64_t price = 0, qty = 0;
            if (!read_u64_be(payloadBytes, off, matchId)) return nullptr;
            if (!read_i64_be(payloadBytes, off, price)) return nullptr;
            if (!read_i64_be(payloadBytes, off, qty)) return nullptr;

            return std::make_unique<FillEvent>(clientId, orderId, matchId, price, qty);
        }
        case EventType::Reject: {
            if (!need(1)) return nullptr;
            auto reason = static_cast<RejectReason>(payloadBytes[off++]);
            if (reason != RejectReason::InvalidQuantity && reason != RejectReason::InvalidPrice && reason != RejectReason::UnknownSymbol && reason != RejectReason::UnknownOrderId && reason != RejectReason::NotOwner && reason != RejectReason::InternalError) return nullptr;
            return std::make_unique<RejectEvent>(clientId, orderId, reason);
        }
        case EventType::Ack: {
            return std::make_unique<AckEvent>(clientId, orderId);
        }
        default:
            return nullptr;
    }
}