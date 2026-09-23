#include "OrderBook.hpp"
#include <algorithm>

OrderBook::OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

std::vector<Trade> OrderBook::addOrder(Order order) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<Trade> generated;
    if (order.side == Side::BUY) {
        generated = matchBuy(order);
    } else {
        generated = matchSell(order);
    }

    // If a LIMIT order still has quantity left after matching, rest it on the book.
    // MARKET orders never rest — any unfilled remainder is dropped (would be
    // "killed" in IOC/FOK semantics; documented simplification).
    if (order.quantity > 0 && order.type == OrderType::LIMIT) {
        insertResting(order);
    }

    return generated;
}

std::vector<Trade> OrderBook::matchBuy(Order& incoming) {
    std::vector<Trade> generated;

    while (incoming.quantity > 0 && !asks_.empty()) {
        auto bestLevelIt = asks_.begin();
        double bestPrice = bestLevelIt->first;

        // Limit price check: only cross if incoming is willing to pay bestPrice
        if (incoming.type == OrderType::LIMIT && incoming.price < bestPrice) {
            break; // best ask is more expensive than buyer will pay
        }

        PriceLevel& level = bestLevelIt->second;
        while (incoming.quantity > 0 && !level.empty()) {
            Order& resting = level.front();
            uint64_t fillQty = std::min(incoming.quantity, resting.quantity);

            Trade t;
            t.tradeId = nextTradeId_++;
            t.buyOrderId = incoming.id;
            t.sellOrderId = resting.id;
            t.symbol = symbol_;
            t.price = bestPrice; // trades execute at the resting order's price
            t.quantity = fillQty;
            t.timestamp = incoming.timestamp;
            trades_.push_back(t);
            generated.push_back(t);

            incoming.quantity -= fillQty;
            resting.quantity -= fillQty;

            if (resting.quantity == 0) {
                resting.status = OrderStatus::FILLED;
                orderLookup_.erase(resting.id);
                level.pop_front();
            } else {
                resting.status = OrderStatus::PARTIALLY_FILLED;
            }
        }

        if (level.empty()) {
            asks_.erase(bestLevelIt);
        }
    }

    incoming.status = incoming.quantity == 0
        ? OrderStatus::FILLED
        : (incoming.quantity < incoming.originalQuantity ? OrderStatus::PARTIALLY_FILLED
                                                           : OrderStatus::NEW);
    return generated;
}

std::vector<Trade> OrderBook::matchSell(Order& incoming) {
    std::vector<Trade> generated;

    while (incoming.quantity > 0 && !bids_.empty()) {
        auto bestLevelIt = bids_.begin();
        double bestPrice = bestLevelIt->first;

        if (incoming.type == OrderType::LIMIT && incoming.price > bestPrice) {
            break; // best bid is lower than seller will accept
        }

        PriceLevel& level = bestLevelIt->second;
        while (incoming.quantity > 0 && !level.empty()) {
            Order& resting = level.front();
            uint64_t fillQty = std::min(incoming.quantity, resting.quantity);

            Trade t;
            t.tradeId = nextTradeId_++;
            t.buyOrderId = resting.id;
            t.sellOrderId = incoming.id;
            t.symbol = symbol_;
            t.price = bestPrice;
            t.quantity = fillQty;
            t.timestamp = incoming.timestamp;
            trades_.push_back(t);
            generated.push_back(t);

            incoming.quantity -= fillQty;
            resting.quantity -= fillQty;

            if (resting.quantity == 0) {
                resting.status = OrderStatus::FILLED;
                orderLookup_.erase(resting.id);
                level.pop_front();
            } else {
                resting.status = OrderStatus::PARTIALLY_FILLED;
            }
        }

        if (level.empty()) {
            bids_.erase(bestLevelIt);
        }
    }

    incoming.status = incoming.quantity == 0
        ? OrderStatus::FILLED
        : (incoming.quantity < incoming.originalQuantity ? OrderStatus::PARTIALLY_FILLED
                                                           : OrderStatus::NEW);
    return generated;
}

void OrderBook::insertResting(Order order) {
    if (order.side == Side::BUY) {
        auto& level = bids_[order.price];
        level.push_back(order);
        auto it = std::prev(level.end());
        orderLookup_[order.id] = {Side::BUY, order.price, it};
    } else {
        auto& level = asks_[order.price];
        level.push_back(order);
        auto it = std::prev(level.end());
        orderLookup_[order.id] = {Side::SELL, order.price, it};
    }
}

bool OrderBook::cancelOrder(uint64_t orderId) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto found = orderLookup_.find(orderId);
    if (found == orderLookup_.end()) return false;

    const OrderLocation& loc = found->second;

    if (loc.side == Side::BUY) {
        auto levelIt = bids_.find(loc.price);
        if (levelIt != bids_.end()) {
            levelIt->second.erase(loc.it);
            if (levelIt->second.empty()) bids_.erase(levelIt);
        }
    } else {
        auto levelIt = asks_.find(loc.price);
        if (levelIt != asks_.end()) {
            levelIt->second.erase(loc.it);
            if (levelIt->second.empty()) asks_.erase(levelIt);
        }
    }

    orderLookup_.erase(found);
    return true;
}

OrderBookSnapshot OrderBook::getSnapshot(int depth) const {
    std::lock_guard<std::mutex> lock(mutex_);

    OrderBookSnapshot snap;
    snap.symbol = symbol_;

    int count = 0;
    for (const auto& [price, level] : bids_) {
        if (count++ >= depth) break;
        uint64_t qty = 0;
        for (const auto& o : level) qty += o.quantity;
        snap.bids.push_back({price, qty, static_cast<int>(level.size())});
    }

    count = 0;
    for (const auto& [price, level] : asks_) {
        if (count++ >= depth) break;
        uint64_t qty = 0;
        for (const auto& o : level) qty += o.quantity;
        snap.asks.push_back({price, qty, static_cast<int>(level.size())});
    }

    return snap;
}
