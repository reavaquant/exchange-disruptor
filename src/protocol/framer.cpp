#include "protocol/framer.h"
#include <stdint.h>
#include <vector>

Framer::Framer() {}

std::vector<std::uint8_t> frame(const std::vector<std::uint8_t>& payloadBytes) { 
    uint32_t len = payloadBytes.size();
    std::vector<std::uint8_t> framedBytes;
    framedBytes.resize(4 + len);
    //big endian (network byte order)
    framedBytes[0] = static_cast<std::uint8_t>(len & 0xFF);
    framedBytes[1] = static_cast<std::uint8_t>((len >> 8) & 0xFF);
    framedBytes[2] = static_cast<std::uint8_t>((len >> 16) & 0xFF);
    framedBytes[3] = static_cast<std::uint8_t>((len >> 24) & 0xFF);
    std::copy(payloadBytes.begin(), payloadBytes.end(), framedBytes.begin() + 4);
    return framedBytes;
}

std::vector<std::vector<uint8_t>> consume(const std::vector<std::uint8_t>& buffer) {
    return std::vector<std::vector<uint8_t>>{};
}