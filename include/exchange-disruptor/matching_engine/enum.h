#ifndef ENUM_H
#define ENUM_H

enum class Side {
    Buy,
    Sell
};

enum class CommandType {
    Limit,
    Market,
    Cancel
};

enum class EventType {
    Ack,
    Reject,
    Fill,
};

enum class RejectReason {
    InvalidQuantity,
    InvalidPrice,
    InvalidSide,
    UnknownSymbol,
    UnknownOrderId,
    DuplicateOrderId,
    NotOwner,
    InternalError,
};


#endif // ENUM_H