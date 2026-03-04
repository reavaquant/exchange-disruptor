#ifndef ENUM_H
#define ENUM_H

#include <cstdint>

enum class Side : uint8_t {
    Buy=1,
    Sell=2
};

enum class CommandType : uint8_t {
    Limit=1,
    Market=2,
    Cancel=3
};

enum class EventType : uint8_t {
    Ack=1,
    Reject=2,
    Fill=3,
};

enum class RejectReason : uint8_t {
    InvalidQuantity=1,
    InvalidPrice=2,
    UnknownSymbol=3,
    UnknownOrderId=4,
    NotOwner=5,
    InternalError=6,
};


#endif // ENUM_H
