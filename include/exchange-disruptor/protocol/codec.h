#ifndef CODEC_H
#define CODEC_H

#include <stdint.h>
#include <vector>

#include "matching_engine/command/command.h"
#include "matching_engine/event/event.h"

class Codec {
public:
    Codec();

    std::vector<std::uint8_t> encodeCommand(const Command& cmd);
    std::vector<std::uint8_t> encodeEvent(const Event& event);

    std::unique_ptr<Command> decodeCommand(const std::vector<std::uint8_t>& payloadBytes);
    std::unique_ptr<Event> decodeEvent(const std::vector<std::uint8_t>& payloadBytes);
};


#endif // CODEC_H