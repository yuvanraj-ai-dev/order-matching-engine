// Standalone CLI demo of the matching engine (useful for quick manual testing
// without spinning up the whole backend/frontend stack).
#include "../include/OrderBook.hpp"
#include <iostream>
#include <chrono>

int64_t now() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

void printSnapshot(const OrderBookSnapshot& snap) {
    std::cout << "\n=== Order Book: " << snap.symbol << " ===\n";
    std::cout << "ASKS (best first):\n";
    for (auto it = snap.asks.rbegin(); it != snap.asks.rend(); ++it) {
        std::cout << "  " << it->price << "  x" << it->totalQuantity
                   << "  (" << it->orderCount << " orders)\n";
    }
    std::cout << "  ------\n";
    std::cout << "BIDS (best first):\n";
    for (const auto& lvl : snap.bids) {
        std::cout << "  " << lvl.price << "  x" << lvl.totalQuantity
                   << "  (" << lvl.orderCount << " orders)\n";
    }
}

int main() {
    OrderBook book("AAPL");

    std::cout << "Placing resting sell orders...\n";
    book.addOrder(Order(1, "AAPL", Side::SELL, OrderType::LIMIT, 101.0, 10, now()));
    book.addOrder(Order(2, "AAPL", Side::SELL, OrderType::LIMIT, 100.5, 5, now()));

    std::cout << "Placing resting buy orders...\n";
    book.addOrder(Order(3, "AAPL", Side::BUY, OrderType::LIMIT, 99.0, 8, now()));
    book.addOrder(Order(4, "AAPL", Side::BUY, OrderType::LIMIT, 98.5, 12, now()));

    printSnapshot(book.getSnapshot());

    std::cout << "\nIncoming aggressive buy order: BUY 12 @ 101.0\n";
    auto trades = book.addOrder(Order(5, "AAPL", Side::BUY, OrderType::LIMIT, 101.0, 12, now()));

    std::cout << "Trades generated: " << trades.size() << "\n";
    for (const auto& t : trades) {
        std::cout << "  Trade #" << t.tradeId << ": " << t.quantity
                   << " @ " << t.price << " (buy=" << t.buyOrderId
                   << ", sell=" << t.sellOrderId << ")\n";
    }

    printSnapshot(book.getSnapshot());

    std::cout << "\nCancelling order #4...\n";
    book.cancelOrder(4);
    printSnapshot(book.getSnapshot());

    return 0;
}
