#include "protocol/codec.h"

Codec::Codec() {}

std::vector<std::uint8_t> Codec::encodeCommand(const Command& cmd) {
    // encode command attributes
    std::vector<std::uint8_t> payloadBytes;
    uint64_t clientId = cmd.getClientId();
    uint64_t orderId = cmd.getOrderId();
    std::string symbol = cmd.getSymbol();

    return {};
}