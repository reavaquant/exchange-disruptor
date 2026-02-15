#ifndef REJECT_EXCEPTION_H
#define REJECT_EXCEPTION_H

#include <exception>
#include "matching_engine/enum.h"

class RejectException : public std::exception {
public:
    RejectException(RejectReason reason);
    const char* what() const noexcept override;
    const RejectReason getReason() const;
private:
    RejectReason _reason;
};
#endif // REJECT_EXCEPTION_H