#pragma once
#include <string>
#include <cstdint>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };
enum class OrderStatus { NEW, PARTIALLY_FILLED, FILLED, CANCELLED };

struct Order {
    uint64_t id;
    std::string symbol;
    Side side;
    OrderType type;
    double price;        // ignored for MARKET orders
    uint64_t quantity;   // remaining quantity
    uint64_t originalQuantity;
    int64_t timestamp;   // nanoseconds since epoch, used for price-time priority
    OrderStatus status;

    Order() = default;

    Order(uint64_t id_, std::string symbol_, Side side_, OrderType type_,
          double price_, uint64_t quantity_, int64_t timestamp_)
        : id(id_), symbol(std::move(symbol_)), side(side_), type(type_),
          price(price_), quantity(quantity_), originalQuantity(quantity_),
          timestamp(timestamp_), status(OrderStatus::NEW) {}
};

struct Trade {
    uint64_t tradeId;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    std::string symbol;
    double price;
    uint64_t quantity;
    int64_t timestamp;
};
