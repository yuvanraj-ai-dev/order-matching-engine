#pragma once
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include "Order.hpp"

// Represents one price level in the book: a FIFO queue of resting orders.
using PriceLevel = std::list<Order>;

struct BookLevel {
    double price;
    uint64_t totalQuantity;
    int orderCount;
};

struct OrderBookSnapshot {
    std::string symbol;
    std::vector<BookLevel> bids; // best (highest) first
    std::vector<BookLevel> asks; // best (lowest) first
};

class OrderBook {
public:
    explicit OrderBook(std::string symbol);

    // Adds a new order, attempts to match it immediately.
    // Returns the list of trades generated.
    std::vector<Trade> addOrder(Order order);

    // Cancels a resting order by id. Returns true if it was found & removed.
    bool cancelOrder(uint64_t orderId);

    // Returns top N levels of the book for display.
    OrderBookSnapshot getSnapshot(int depth = 10) const;

    const std::vector<Trade>& getTradeHistory() const { return trades_; }

    const std::string& symbol() const { return symbol_; }

    size_t totalRestingOrders() const { return orderLookup_.size(); }

private:
    std::string symbol_;

    // Bids sorted highest price first, asks sorted lowest price first.
    std::map<double, PriceLevel, std::greater<double>> bids_;
    std::map<double, PriceLevel> asks_;

    // Fast O(1) lookup for cancellation: orderId -> (side, price, iterator)
    struct OrderLocation {
        Side side;
        double price;
        std::list<Order>::iterator it;
    };
    std::unordered_map<uint64_t, OrderLocation> orderLookup_;

    std::vector<Trade> trades_;
    uint64_t nextTradeId_ = 1;

    mutable std::mutex mutex_; // coarse-grained lock for thread safety

    std::vector<Trade> matchBuy(Order& incoming);
    std::vector<Trade> matchSell(Order& incoming);
    void insertResting(Order order);
};
