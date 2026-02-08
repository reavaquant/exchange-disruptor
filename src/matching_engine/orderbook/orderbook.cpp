#include "orderbook.h"

OrderBook::OrderBook(std::string symbol) : _symbol(symbol), _bidBook(BookSide::Bid), _askBook(BookSide::Ask) {}

const Book& OrderBook::getBidBook() const { return _bidBook; }
const Book& OrderBook::getAskBook() const { return _askBook; }
std::string OrderBook::getSymbol() const { return _symbol; }