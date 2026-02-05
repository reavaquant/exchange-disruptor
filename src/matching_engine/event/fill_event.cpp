#include "fill_event.h"

FillEvent::FillEvent(uint64_t clientId, uint64_t orderId, uint64_t matchId, double price, double qty) : Event(clientId, orderId), _matchId(matchId), _price(price), _qty(qty) {}

EventType FillEvent::getType() const { return EventType::Fill; }

uint64_t FillEvent::getMatchId() const { return _matchId; }
double FillEvent::getPrice() const { return _price; }
double FillEvent::getQty() const { return _qty; }