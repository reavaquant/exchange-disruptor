#include "matching_engine/reject_exception.h"

RejectException::RejectException(RejectReason reason) : _reason(reason) {}

const char* RejectException::what() const noexcept {
    switch (_reason) {
        case RejectReason::InvalidQuantity:
            return "Invalid quantity";
        case RejectReason::InvalidPrice:
            return "Invalid price";
        case RejectReason::UnknownSymbol:
            return "Unknown symbol";
        case RejectReason::UnknownOrderId:
            return "Unknown order ID";
        case RejectReason::NotOwner:
            return "Not owner of the order";
        case RejectReason::InternalError:
            return "Internal error";
        default:
            return "Unknown reject reason";
    }
}

const RejectReason RejectException::getReason() const {
    return _reason;
}