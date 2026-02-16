#ifndef FRAMER_H
#define FRAMER_H

#include <stdint.h>
#include <vector>

class Framer {
public:
    Framer();

    std::vector<std::uint8_t> frame(const std::vector<std::uint8_t>& payloadBytes);
    std::vector<std::vector<uint8_t>> consume(const std::vector<std::uint8_t>& buffer);
};

#endif // FRAMER_H