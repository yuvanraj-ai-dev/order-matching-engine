#include "BookManager.h"
#include "Persistence.h"
#include "RedisPublisher.h"
#include "JsonSerializers.h"
#include <chrono>

BookManager& BookManager::instance() {
    static BookManager mgr;
    return mgr;
}

OrderBook& BookManager::bookFor(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(booksMutex_);
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        auto [inserted, _] = books_.emplace(symbol, std::make_unique<OrderBook>(symbol));
        return *inserted->second;
    }
    return *it->second;
}

uint64_t BookManager::nextOrderId() {
    return nextOrderId_.fetch_add(1);
}

BookManager::PlaceResult BookManager::placeOrder(const std::string& symbol,
                                                  Side side,
                                                  OrderType type,
                                                  double price,
                                                  uint64_t quantity) {
    auto& book = bookFor(symbol);

    int64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();

    Order order(nextOrderId(), symbol, side, type, price, quantity, ts);

    // Persist the incoming order in its NEW state before matching, so it's
    // recorded even if the process crashes mid-match (rare, but this is the
    // kind of durability question that comes up in interviews).
    Persistence::saveOrder(order);

    auto trades = book.addOrder(order);

    // order.status/quantity were mutated in place by addOrder; reflect that.
    Persistence::updateOrderStatus(order.id, order.status, order.quantity);

    for (const auto& t : trades) {
        Persistence::saveTrade(t);
    }

    broadcastBookUpdate(symbol);
    if (!trades.empty()) {
        broadcastTrades(symbol, trades);
    }

    return {order, trades};
}

bool BookManager::cancelOrder(const std::string& symbol, uint64_t orderId) {
    auto& book = bookFor(symbol);
    bool ok = book.cancelOrder(orderId);
    if (ok) {
        Persistence::updateOrderStatus(orderId, OrderStatus::CANCELLED, 0);
        broadcastBookUpdate(symbol);
    }
    return ok;
}

OrderBookSnapshot BookManager::getSnapshot(const std::string& symbol, int depth) {
    return bookFor(symbol).getSnapshot(depth);
}

std::vector<Trade> BookManager::getTradeHistory(const std::string& symbol) {
    return bookFor(symbol).getTradeHistory();
}

void BookManager::subscribe(const std::string& symbol, const drogon::WebSocketConnectionPtr& conn) {
    std::lock_guard<std::mutex> lock(subsMutex_);
    subscribers_[symbol].insert(conn);
    connSymbol_[conn] = symbol;
}

void BookManager::unsubscribe(const drogon::WebSocketConnectionPtr& conn) {
    std::lock_guard<std::mutex> lock(subsMutex_);
    auto it = connSymbol_.find(conn);
    if (it == connSymbol_.end()) return;
    auto& set = subscribers_[it->second];
    set.erase(conn);
    if (set.empty()) subscribers_.erase(it->second);
    connSymbol_.erase(it);
}

void BookManager::broadcastBookUpdate(const std::string& symbol) {
    auto snap = getSnapshot(symbol, 10);
    Json::Value j = Json2::snapshotToJson(snap);
    j["channel"] = "orderbook";
    std::string payload = Json2::toCompactString(j);

    RedisPublisher::publishBookUpdate(symbol, payload);

    std::lock_guard<std::mutex> lock(subsMutex_);
    auto it = subscribers_.find(symbol);
    if (it == subscribers_.end()) return;
    for (const auto& conn : it->second) {
        if (conn->connected()) conn->send(payload);
    }
}

void BookManager::broadcastTrades(const std::string& symbol, const std::vector<Trade>& trades) {
    Json::Value arr(Json::arrayValue);
    for (const auto& t : trades) arr.append(Json2::tradeToJson(t));
    Json::Value j;
    j["channel"] = "trades";
    j["symbol"] = symbol;
    j["trades"] = arr;
    std::string payload = Json2::toCompactString(j);

    for (const auto& t : trades) {
        RedisPublisher::publishTrade(symbol, Json2::toCompactString(Json2::tradeToJson(t)));
    }

    std::lock_guard<std::mutex> lock(subsMutex_);
    auto it = subscribers_.find(symbol);
    if (it == subscribers_.end()) return;
    for (const auto& conn : it->second) {
        if (conn->connected()) conn->send(payload);
    }
}
