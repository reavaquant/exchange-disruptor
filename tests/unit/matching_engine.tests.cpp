#include "matching_engine/matching_engine.h"
#include "matching_engine/orderbook/orderbook.h"
#include "matching_engine/order/order.h"
#include "matching_engine/command/limit_command.h"
#include "matching_engine/command/market_command.h"
#include "matching_engine/command/cancel_command.h"
#include "matching_engine/event/fill_event.h"
#include "matching_engine/event/reject_event.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

int g_total_tests = 0;
int g_failed_tests = 0;
int g_total_checks = 0;
int g_failed_checks = 0;
const char* g_current_test = "";

void check(bool condition, const char* expr, const char* file, int line) {
    ++g_total_checks;
    if (!condition) {
        ++g_failed_checks;
        std::cerr << "[FAIL] " << g_current_test << ": " << expr
                  << " (" << file << ":" << line << ")\n";
    }
}

template <typename A, typename B>
void check_eq(const A& a, const B& b, const char* expr_a, const char* expr_b, const char* file, int line) {
    ++g_total_checks;
    bool equal = false;
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        // Avoid -Wsign-compare and handle signed/unsigned safely.
        equal = std::cmp_equal(a, b);
    } else {
        equal = (a == b);
    }
    if (!equal) {
        ++g_failed_checks;
        std::cerr << "[FAIL] " << g_current_test << ": " << expr_a << " == " << expr_b
                  << " (got " << a << " vs " << b << ")"
                  << " (" << file << ":" << line << ")\n";
    }
}

#define CHECK(expr) check((expr), #expr, __FILE__, __LINE__)
#define CHECK_EQ(a, b) check_eq((a), (b), #a, #b, __FILE__, __LINE__)

int count_type(const std::vector<std::unique_ptr<Event>>& events, EventType type) {
    int count = 0;
    for (const auto& event : events) {
        if (event->getType() == type) {
            ++count;
        }
    }
    return count;
}

const FillEvent* first_fill_for(const std::vector<std::unique_ptr<Event>>& events, uint64_t orderId) {
    for (const auto& event : events) {
        if (event->getType() != EventType::Fill) continue;
        const auto* fill = static_cast<const FillEvent*>(event.get());
        if (fill->getOrderId() == orderId) return fill;
    }
    return nullptr;
}

int first_fill_pos(const std::vector<std::unique_ptr<Event>>& events, uint64_t orderId) {
    for (size_t i = 0; i < events.size(); ++i) {
        if (events[i]->getType() != EventType::Fill) continue;
        const auto* fill = static_cast<const FillEvent*>(events[i].get());
        if (fill->getOrderId() == orderId) return static_cast<int>(i);
    }
    return -1;
}

struct FillStats {
    int count = 0;
    int64_t qty_sum = 0;
    bool all_prices_match = true;
};

FillStats fill_stats(const std::vector<std::unique_ptr<Event>>& events, uint64_t orderId, std::optional<int64_t> expected_price = std::nullopt) {
    FillStats stats{};
    for (const auto& event : events) {
        if (event->getType() != EventType::Fill) continue;
        const auto* fill = static_cast<const FillEvent*>(event.get());
        if (fill->getOrderId() != orderId) continue;

        ++stats.count;
        stats.qty_sum += fill->getQty();
        if (expected_price && fill->getPrice() != *expected_price) {
            stats.all_prices_match = false;
        }
    }
    return stats;
}

const RejectEvent* find_reject(const std::vector<std::unique_ptr<Event>>& events) {
    for (const auto& event : events) {
        if (event->getType() == EventType::Reject) {
            return static_cast<const RejectEvent*>(event.get());
        }
    }
    return nullptr;
}

void expect_reject_reason(const std::vector<std::unique_ptr<Event>>& events, RejectReason reason) {
    CHECK_EQ(count_type(events, EventType::Reject), 1);
    const auto* reject = find_reject(events);
    CHECK(reject != nullptr);
    if (reject) CHECK(reject->getReason() == reason);
}

void run_test(const char* name, void (*fn)()) {
    ++g_total_tests;
    g_current_test = name;
    const int failed_before = g_failed_checks;
    fn();
    if (g_failed_checks == failed_before) {
        std::cout << "[PASS] " << name << "\n";
    } else {
        ++g_failed_tests;
        std::cout << "[FAIL] " << name << "\n";
    }
}

void test_limit_no_crossing_updates_book() {
    OrderBook book("ACME");
    Order order(1, 101, "ACME", Side::Buy, 100, 10);
    auto reject = book.addLimit(order);
    CHECK(!reject.has_value());
    CHECK(book.hasOrderId(101));
    CHECK(book.topBidPrice().has_value());
    CHECK_EQ(book.topBidPrice().value(), 100);
    const Order* top_bid = book.peekBid();
    CHECK(top_bid != nullptr);
    if (top_bid) {
        CHECK_EQ(top_bid->getOrderId(), 101);
        CHECK_EQ(top_bid->getQtyRemaining(), 10);
    }
}

void test_limit_crossing_full_fill() {
    MatchingEngine engine("ACME");
    auto seed_events = engine.process(LimitCommand(1, 1, "ACME", Side::Sell, 100, 5));
    CHECK_EQ(count_type(seed_events, EventType::Ack), 1);
    CHECK_EQ(count_type(seed_events, EventType::Reject), 0);

    auto events = engine.process(LimitCommand(2, 2, "ACME", Side::Buy, 100, 5));
    CHECK_EQ(count_type(events, EventType::Fill), 2);
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Reject), 0);

    {
        auto s1 = fill_stats(events, 1u, 100);
        auto s2 = fill_stats(events, 2u, 100);
        CHECK_EQ(s1.count, 1);
        CHECK_EQ(s2.count, 1);
        CHECK_EQ(s1.qty_sum, 5);
        CHECK_EQ(s2.qty_sum, 5);
        CHECK(s1.all_prices_match);
        CHECK(s2.all_prices_match);
    }

    const auto* f1 = first_fill_for(events, 1u);
    const auto* f2 = first_fill_for(events, 2u);
    CHECK(f1 != nullptr);
    CHECK(f2 != nullptr);
    if (f1 && f2) CHECK_EQ(f1->getMatchId(), f2->getMatchId());

    // Both orders should be gone from the book after a full match.
    expect_reject_reason(engine.process(CancelCommand(1, 1, "ACME")), RejectReason::UnknownOrderId);
}

void test_limit_crossing_partial_fill_remainder_in_book() {
    MatchingEngine engine("ACME");
    engine.process(LimitCommand(1, 1, "ACME", Side::Sell, 100, 4));

    auto events = engine.process(LimitCommand(2, 2, "ACME", Side::Buy, 100, 10));
    CHECK_EQ(count_type(events, EventType::Fill), 2);
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Reject), 0);

    {
        auto s1 = fill_stats(events, 1u, 100);
        auto s2 = fill_stats(events, 2u, 100);
        CHECK_EQ(s1.count, 1);
        CHECK_EQ(s2.count, 1);
        CHECK_EQ(s1.qty_sum, 4);
        CHECK_EQ(s2.qty_sum, 4);
        CHECK(s1.all_prices_match);
        CHECK(s2.all_prices_match);
    }

    // The remaining 6 qty from orderId=2 should be resting in the book at 100.
    auto sweep = engine.process(MarketCommand(3, 3, "ACME", Side::Sell, 6));
    CHECK_EQ(count_type(sweep, EventType::Ack), 1);
    CHECK_EQ(count_type(sweep, EventType::Reject), 0);
    CHECK_EQ(count_type(sweep, EventType::Fill), 2);
    {
        auto s2 = fill_stats(sweep, 2u, 100);
        auto s3 = fill_stats(sweep, 3u, 100);
        CHECK_EQ(s2.count, 1);
        CHECK_EQ(s3.count, 1);
        CHECK_EQ(s2.qty_sum, 6);
        CHECK_EQ(s3.qty_sum, 6);
        CHECK(s2.all_prices_match);
        CHECK(s3.all_prices_match);
    }

    // After sweeping the remainder, orderId=2 should be gone.
    expect_reject_reason(engine.process(CancelCommand(2, 2, "ACME")), RejectReason::UnknownOrderId);
}

void test_market_sweeps_multiple_levels() {
    MatchingEngine engine("ACME");
    engine.process(LimitCommand(1, 1, "ACME", Side::Sell, 100, 3));
    engine.process(LimitCommand(2, 2, "ACME", Side::Sell, 101, 4));
    engine.process(LimitCommand(3, 3, "ACME", Side::Sell, 102, 5));

    auto events = engine.process(MarketCommand(9, 99, "ACME", Side::Buy, 10));
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Fill), 6);
    CHECK_EQ(count_type(events, EventType::Reject), 0);
    {
        auto s1 = fill_stats(events, 1u, 100);
        auto s2 = fill_stats(events, 2u, 101);
        auto s3 = fill_stats(events, 3u, 102);
        auto taker = fill_stats(events, 99u);
        CHECK_EQ(s1.qty_sum, 3);
        CHECK_EQ(s2.qty_sum, 4);
        CHECK_EQ(s3.qty_sum, 3);
        CHECK_EQ(taker.qty_sum, 10);
        CHECK(s1.all_prices_match);
        CHECK(s2.all_prices_match);
        CHECK(s3.all_prices_match);
    }

    // Price-time priority across levels for a buy market order: 100 then 101 then 102.
    CHECK(first_fill_pos(events, 1u) != -1);
    CHECK(first_fill_pos(events, 2u) != -1);
    CHECK(first_fill_pos(events, 3u) != -1);
    CHECK(first_fill_pos(events, 1u) < first_fill_pos(events, 2u));
    CHECK(first_fill_pos(events, 2u) < first_fill_pos(events, 3u));

    // OrderId=3 should have 2 qty remaining at 102.
    auto finish = engine.process(MarketCommand(10, 100, "ACME", Side::Buy, 2));
    CHECK_EQ(count_type(finish, EventType::Ack), 1);
    CHECK_EQ(count_type(finish, EventType::Reject), 0);
    CHECK_EQ(count_type(finish, EventType::Fill), 2);
    {
        auto s3 = fill_stats(finish, 3u, 102);
        auto s100 = fill_stats(finish, 100u, 102);
        CHECK_EQ(s3.qty_sum, 2);
        CHECK_EQ(s100.qty_sum, 2);
        CHECK(s3.all_prices_match);
        CHECK(s100.all_prices_match);
    }

    expect_reject_reason(engine.process(CancelCommand(3, 3, "ACME")), RejectReason::UnknownOrderId);
}

void test_fifo_same_price() {
    MatchingEngine engine("ACME");
    engine.process(LimitCommand(1, 10, "ACME", Side::Sell, 100, 5));
    engine.process(LimitCommand(2, 11, "ACME", Side::Sell, 100, 5));

    auto events = engine.process(MarketCommand(9, 99, "ACME", Side::Buy, 7));
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Reject), 0);
    CHECK_EQ(count_type(events, EventType::Fill), 4);
    {
        // FIFO at same price: orderId=10 must be fully consumed before orderId=11.
        auto s10 = fill_stats(events, 10u, 100);
        auto s11 = fill_stats(events, 11u, 100);
        auto taker = fill_stats(events, 99u, 100);
        CHECK_EQ(s10.qty_sum, 5);
        CHECK_EQ(s11.qty_sum, 2);
        CHECK_EQ(taker.qty_sum, 7);
        CHECK(s10.all_prices_match);
        CHECK(s11.all_prices_match);
        CHECK(taker.all_prices_match);
    }

    // FIFO at same price: maker fill for orderId=10 must appear before maker fill for orderId=11.
    CHECK(first_fill_pos(events, 10u) != -1);
    CHECK(first_fill_pos(events, 11u) != -1);
    CHECK(first_fill_pos(events, 10u) < first_fill_pos(events, 11u));

    // Remaining qty (3) on orderId=11 should still be there.
    auto finish = engine.process(MarketCommand(10, 98, "ACME", Side::Buy, 3));
    CHECK_EQ(count_type(finish, EventType::Fill), 2);
    {
        auto s11 = fill_stats(finish, 11u, 100);
        auto s98 = fill_stats(finish, 98u, 100);
        CHECK_EQ(s11.qty_sum, 3);
        CHECK_EQ(s98.qty_sum, 3);
        CHECK(s11.all_prices_match);
        CHECK(s98.all_prices_match);
    }

    expect_reject_reason(engine.process(CancelCommand(2, 11, "ACME")), RejectReason::UnknownOrderId);
}

void test_cancel_existing_order() {
    MatchingEngine engine("ACME");
    engine.process(LimitCommand(1, 1, "ACME", Side::Buy, 99, 5));

    auto events = engine.process(CancelCommand(1, 1, "ACME"));
    CHECK_EQ(count_type(events, EventType::Ack), 1);
    CHECK_EQ(count_type(events, EventType::Reject), 0);
}

void test_cancel_filled_order_reject() {
    MatchingEngine engine("ACME");
    engine.process(LimitCommand(1, 1, "ACME", Side::Sell, 100, 5));
    engine.process(LimitCommand(2, 2, "ACME", Side::Buy, 100, 5));

    expect_reject_reason(engine.process(CancelCommand(1, 1, "ACME")), RejectReason::UnknownOrderId);
}

void test_cancel_unknown_order_reject() {
    MatchingEngine engine("ACME");
    expect_reject_reason(engine.process(CancelCommand(1, 999, "ACME")), RejectReason::UnknownOrderId);
}

} // namespace

int main() {
    run_test("limit sans crossing => book mis a jour", test_limit_no_crossing_updates_book);
    run_test("limit crossing => fill total", test_limit_crossing_full_fill);
    run_test("limit crossing => fill partiel + reste au book", test_limit_crossing_partial_fill_remainder_in_book);
    run_test("market balaye plusieurs niveaux", test_market_sweeps_multiple_levels);
    run_test("FIFO a prix egal", test_fifo_same_price);
    run_test("cancel ordre existant", test_cancel_existing_order);
    run_test("cancel ordre deja rempli => reject", test_cancel_filled_order_reject);
    run_test("cancel orderId inconnu => reject", test_cancel_unknown_order_reject);

    std::cout << "Tests: " << (g_total_tests - g_failed_tests) << "/" << g_total_tests
              << " passed, Checks: " << (g_total_checks - g_failed_checks)
              << "/" << g_total_checks << "\n";

    return g_failed_tests == 0 ? 0 : 1;
}
