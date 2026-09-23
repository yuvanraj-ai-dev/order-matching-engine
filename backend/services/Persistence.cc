#include "Persistence.h"
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

using namespace drogon;
using namespace drogon::orm;

namespace {
std::string sideToStr(Side s) { return s == Side::BUY ? "BUY" : "SELL"; }
std::string typeToStr(OrderType t) { return t == OrderType::LIMIT ? "LIMIT" : "MARKET"; }
std::string statusToStr(OrderStatus s) {
    switch (s) {
        case OrderStatus::NEW: return "NEW";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
    }
    return "NEW";
}
}

namespace Persistence {

void saveOrder(const Order& order) {
    auto db = app().getDbClient();
    if (!db) return; // DB not configured -- app still runs, just skips persistence

    *db << "INSERT INTO orders "
           "(id, symbol, side, type, price, quantity, original_quantity, status, created_at) "
           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, to_timestamp(?)) "
           "ON CONFLICT (id) DO NOTHING"
        << static_cast<int64_t>(order.id)
        << order.symbol
        << sideToStr(order.side)
        << typeToStr(order.type)
        << order.price
        << static_cast<int64_t>(order.quantity)
        << static_cast<int64_t>(order.originalQuantity)
        << statusToStr(order.status)
        << static_cast<double>(order.timestamp) / 1e9
        >> [](const Result&) { /* success, nothing to do */ }
        >> [](const DrogonDbException& e) {
               LOG_ERROR << "saveOrder failed: " << e.base().what();
           };
}

void updateOrderStatus(uint64_t orderId, OrderStatus status, uint64_t remainingQty) {
    auto db = app().getDbClient();
    if (!db) return;

    *db << "UPDATE orders SET status = ?, quantity = ? WHERE id = ?"
        << statusToStr(status)
        << static_cast<int64_t>(remainingQty)
        << static_cast<int64_t>(orderId)
        >> [](const Result&) {}
        >> [](const DrogonDbException& e) {
               LOG_ERROR << "updateOrderStatus failed: " << e.base().what();
           };
}

void saveTrade(const Trade& trade) {
    auto db = app().getDbClient();
    if (!db) return;

    *db << "INSERT INTO trades "
           "(trade_id, buy_order_id, sell_order_id, symbol, price, quantity, executed_at) "
           "VALUES (?, ?, ?, ?, ?, ?, to_timestamp(?))"
        << static_cast<int64_t>(trade.tradeId)
        << static_cast<int64_t>(trade.buyOrderId)
        << static_cast<int64_t>(trade.sellOrderId)
        << trade.symbol
        << trade.price
        << static_cast<int64_t>(trade.quantity)
        << static_cast<double>(trade.timestamp) / 1e9
        >> [](const Result&) {}
        >> [](const DrogonDbException& e) {
               LOG_ERROR << "saveTrade failed: " << e.base().what();
           };
}

} // namespace Persistence
