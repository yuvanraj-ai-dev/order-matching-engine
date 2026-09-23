#pragma once
#include <json/json.h>
#include "../../engine/include/Order.hpp"
#include "../../engine/include/OrderBook.hpp"

// Small free functions converting engine domain types to Json::Value / strings.
// Kept separate from the domain types themselves so engine/ has zero
// dependency on jsoncpp or Drogon -- it stays a portable, standalone library.
namespace Json2 {

inline std::string sideStr(Side s) { return s == Side::BUY ? "BUY" : "SELL"; }
inline std::string typeStr(OrderType t) { return t == OrderType::LIMIT ? "LIMIT" : "MARKET"; }
inline std::string statusStr(OrderStatus s) {
    switch (s) {
        case OrderStatus::NEW: return "NEW";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
    }
    return "NEW";
}

inline Json::Value orderToJson(const Order& o) {
    Json::Value j;
    j["id"] = static_cast<Json::UInt64>(o.id);
    j["symbol"] = o.symbol;
    j["side"] = sideStr(o.side);
    j["type"] = typeStr(o.type);
    j["price"] = o.price;
    j["quantity"] = static_cast<Json::UInt64>(o.quantity);
    j["original_quantity"] = static_cast<Json::UInt64>(o.originalQuantity);
    j["status"] = statusStr(o.status);
    j["timestamp"] = static_cast<Json::UInt64>(o.timestamp);
    return j;
}

inline Json::Value tradeToJson(const Trade& t) {
    Json::Value j;
    j["trade_id"] = static_cast<Json::UInt64>(t.tradeId);
    j["buy_order_id"] = static_cast<Json::UInt64>(t.buyOrderId);
    j["sell_order_id"] = static_cast<Json::UInt64>(t.sellOrderId);
    j["symbol"] = t.symbol;
    j["price"] = t.price;
    j["quantity"] = static_cast<Json::UInt64>(t.quantity);
    j["timestamp"] = static_cast<Json::UInt64>(t.timestamp);
    return j;
}

inline Json::Value snapshotToJson(const OrderBookSnapshot& snap) {
    Json::Value j;
    j["symbol"] = snap.symbol;
    Json::Value bids(Json::arrayValue);
    for (const auto& lvl : snap.bids) {
        Json::Value l;
        l["price"] = lvl.price;
        l["total_quantity"] = static_cast<Json::UInt64>(lvl.totalQuantity);
        l["order_count"] = lvl.orderCount;
        bids.append(l);
    }
    Json::Value asks(Json::arrayValue);
    for (const auto& lvl : snap.asks) {
        Json::Value l;
        l["price"] = lvl.price;
        l["total_quantity"] = static_cast<Json::UInt64>(lvl.totalQuantity);
        l["order_count"] = lvl.orderCount;
        asks.append(l);
    }
    j["bids"] = bids;
    j["asks"] = asks;
    return j;
}

inline std::string toCompactString(const Json::Value& j) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, j);
}

} // namespace Json2
