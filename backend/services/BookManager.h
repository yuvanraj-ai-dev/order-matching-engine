#pragma once
#include <drogon/WebSocketConnection.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include "../../engine/include/OrderBook.hpp"

// Owns one OrderBook per symbol and the set of WebSocket connections
// subscribed to each symbol's live feed. This is the single point where
// REST controllers and the WebSocket controller both talk to the engine,
// so book state and broadcast state stay consistent.
//
// Concurrency note: OrderBook itself is internally thread-safe (mutex-guarded).
// This class adds a second, coarser lock just for the subscriber registry,
// which is a much smaller critical section than matching itself.
class BookManager {
public:
    static BookManager& instance();

    // Places an order on the given symbol's book, persists it + resulting
    // trades, and broadcasts an updated snapshot/trade feed to subscribers.
    struct PlaceResult {
        Order order;
        std::vector<Trade> trades;
    };
    PlaceResult placeOrder(const std::string& symbol,
                            Side side,
                            OrderType type,
                            double price,
                            uint64_t quantity);

    bool cancelOrder(const std::string& symbol, uint64_t orderId);

    OrderBookSnapshot getSnapshot(const std::string& symbol, int depth = 10);

    std::vector<Trade> getTradeHistory(const std::string& symbol);

    uint64_t nextOrderId();

    // WebSocket subscriber management
    void subscribe(const std::string& symbol, const drogon::WebSocketConnectionPtr& conn);
    void unsubscribe(const drogon::WebSocketConnectionPtr& conn);
    void broadcastBookUpdate(const std::string& symbol);
    void broadcastTrades(const std::string& symbol, const std::vector<Trade>& trades);

private:
    BookManager() = default;

    OrderBook& bookFor(const std::string& symbol);

    std::mutex booksMutex_;
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;

    std::mutex subsMutex_;
    // symbol -> set of subscribed connections
    std::unordered_map<std::string, std::unordered_set<drogon::WebSocketConnectionPtr>> subscribers_;
    // reverse index so we can clean up on disconnect in O(1) amortized
    std::unordered_map<drogon::WebSocketConnectionPtr, std::string> connSymbol_;

    std::atomic<uint64_t> nextOrderId_{1};
};
