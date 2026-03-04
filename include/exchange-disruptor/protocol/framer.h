#ifndef FRAMER_H
#define FRAMER_H

#include <cstddef>
#include <stdint.h>
#include <span>
#include <vector>

class Framer {
public:
    Framer();

    std::vector<std::uint8_t> frame(const std::vector<std::uint8_t>& payloadBytes);
    std::vector<std::vector<uint8_t>> consume(std::span<const uint8_t> buffer);
    static constexpr std::size_t getHeaderSize();
private:
    std::vector<uint8_t> _buffer;
    std::size_t _headerPos = 0;
    static constexpr std::size_t kHeaderSize = 4;
};

#endif // FRAMER_H
