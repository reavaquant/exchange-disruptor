#include "protocol/framer.h"
#include <stdint.h>
#include <vector>

Framer::Framer() {}

std::vector<std::uint8_t> Framer::frame(const std::vector<std::uint8_t>& payloadBytes) { 
    uint32_t len = payloadBytes.size();
    std::vector<std::uint8_t> framedBytes;
    framedBytes.resize(4 + len);
    //big endian (network byte order)
    framedBytes[0] = static_cast<uint8_t>((len >> 24) & 0xFF);
    framedBytes[1] = static_cast<uint8_t>((len >> 16) & 0xFF);
    framedBytes[2] = static_cast<uint8_t>((len >> 8) & 0xFF);
    framedBytes[3] = static_cast<uint8_t>(len & 0xFF);
    std::copy(payloadBytes.begin(), payloadBytes.end(), framedBytes.begin() + 4);
    return framedBytes;
}

std::vector<std::vector<uint8_t>> Framer::consume(const std::vector<std::uint8_t>& buffer) { // pas mega optimal : ring buffer+std::span for V2
    _buffer.insert(_buffer.end(), buffer.begin(), buffer.end());
    std::vector<std::vector<uint8_t>> messages;
    while (_buffer.size() - _headerPos >= kHeaderSize) {
        uint32_t len = (uint32_t(_buffer[_headerPos + 0]) << 24) | (uint32_t(_buffer[_headerPos + 1]) << 16) | (uint32_t(_buffer[_headerPos + 2]) <<  8) | (uint32_t(_buffer[_headerPos + 3]));
        if (len > 1024) { //bad frame size
            _buffer.clear();
            _headerPos = 0;
            break;
        }
        if (_buffer.size() - _headerPos < kHeaderSize + len) {
            break;
        }
        std::vector<uint8_t> message(len);
        std::copy(_buffer.begin() + _headerPos + kHeaderSize, _buffer.begin() + _headerPos + kHeaderSize + len, message.begin());
        _headerPos += kHeaderSize + len;  
        if (_headerPos == _buffer.size()) {
            _headerPos = 0;
            _buffer.clear();
        }  
        if (_headerPos > _buffer.size()/2) {
            _buffer.erase(_buffer.begin(), _buffer.begin() + _headerPos);
            _headerPos = 0;
        }
        messages.push_back(message);
    }
    return messages;
}