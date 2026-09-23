// Lightweight self-contained test harness (no external framework needed,
// so this compiles anywhere with just g++ -- no network / package fetch required).
#include "../include/OrderBook.hpp"
#include <iostream>
#include <cassert>
#include <chrono>

int testsRun = 0;
int testsPassed = 0;

#define CHECK(cond) do { \
    testsRun++; \
    if (cond) { testsPassed++; } \
    else { std::cerr << "FAILED: " << #cond << " at line " << __LINE__ << "\n"; } \
} while (0)

int64_t now() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

void test_full_fill() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 10, now()));
    auto trades = book.addOrder(Order(2, "AAPL", Side::BUY, OrderType::LIMIT, 100.0, 10, now()));

    CHECK(trades.size() == 1);
    CHECK(trades[0].quantity == 10);
    CHECK(trades[0].price == 100.0);
    CHECK(book.totalRestingOrders() == 0); // both fully filled, nothing rests
}

void test_partial_fill() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 10, now()));
    auto trades = book.addOrder(Order(2, "AAPL", Side::BUY, OrderType::LIMIT, 100.0, 4, now()));

    CHECK(trades.size() == 1);
    CHECK(trades[0].quantity == 4);

    auto snap = book.getSnapshot();
    CHECK(snap.asks.size() == 1);
    CHECK(snap.asks[0].totalQuantity == 6); // 10 - 4 remaining resting on the book
}

void test_no_match_resting_order() {
    OrderBook book("AAPL");
    auto trades = book.addOrder(Order(1, "AAPL", Side::BUY, OrderType::LIMIT, 99.0, 5, now()));
    CHECK(trades.size() == 0);

    auto snap = book.getSnapshot();
    CHECK(snap.bids.size() == 1);
    CHECK(snap.bids[0].price == 99.0);
    CHECK(snap.bids[0].totalQuantity == 5);
}

void test_price_time_priority_fifo() {
    OrderBook book("AAPL");
    // Two resting sell orders at the SAME price -- first one in should fill first.
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 5, now()));
    book.addOrder(Order(2, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 5, now()));

    auto trades = book.addOrder(Order(3, "AAPL", Side::BUY, OrderType::LIMIT, 100.0, 5, now()));

    CHECK(trades.size() == 1);
    CHECK(trades[0].sellOrderId == 1); // order 1 was resting first -> matched first
}

void test_price_priority_best_price_first() {
    OrderBook book("AAPL");
    // Two sell levels; buyer should match the CHEAPER one first even though
    // it was placed second.
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 101.0, 5, now()));
    book.addOrder(Order(2, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 5, now()));

    auto trades = book.addOrder(Order(3, "AAPL", Side::BUY, OrderType::LIMIT, 101.0, 5, now()));

    CHECK(trades.size() == 1);
    CHECK(trades[0].sellOrderId == 2);
    CHECK(trades[0].price == 100.0);
}

void test_cancel_resting_order() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::BUY, OrderType::LIMIT, 99.0, 5, now()));
    bool cancelled = book.cancelOrder(1);
    CHECK(cancelled == true);
    CHECK(book.totalRestingOrders() == 0);

    auto snap = book.getSnapshot();
    CHECK(snap.bids.size() == 0);
}

void test_cancel_already_filled_order_fails_gracefully() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 10, now()));
    book.addOrder(Order(2, "AAPL", Side::BUY, OrderType::LIMIT, 100.0, 10, now())); // fully fills #1

    bool cancelled = book.cancelOrder(1); // already gone -- must not crash
    CHECK(cancelled == false);
}

void test_market_order_sweeps_book() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 100.0, 5, now()));
    book.addOrder(Order(2, "AAPL", Side::SELL, OrderType::LIMIT, 101.0, 5, now()));

    // Market buy for 8 shares should sweep the best level fully, then partially the next.
    auto trades = book.addOrder(Order(3, "AAPL", Side::BUY, OrderType::MARKET, 0.0, 8, now()));

    CHECK(trades.size() == 2);
    CHECK(trades[0].price == 100.0);
    CHECK(trades[0].quantity == 5);
    CHECK(trades[1].price == 101.0);
    CHECK(trades[1].quantity == 3);
}

void test_no_cross_when_prices_dont_overlap() {
    OrderBook book("AAPL");
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 105.0, 5, now()));
    auto trades = book.addOrder(Order(2, "AAPL", Side::BUY, OrderType::LIMIT, 100.0, 5, now()));

    CHECK(trades.size() == 0); // buyer won't pay 105, so no match; both rest
    auto snap = book.getSnapshot();
    CHECK(snap.bids.size() == 1);
    CHECK(snap.asks.size() == 1);
}

int main() {
    test_full_fill();
    test_partial_fill();
    test_no_match_resting_order();
    test_price_time_priority_fifo();
    test_price_priority_best_price_first();
    test_cancel_resting_order();
    test_cancel_already_filled_order_fails_gracefully();
    test_market_order_sweeps_book();
    test_no_cross_when_prices_dont_overlap();

    std::cout << "\n" << testsPassed << " / " << testsRun << " assertions passed\n";
    return (testsPassed == testsRun) ? 0 : 1;
}
