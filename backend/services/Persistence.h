#pragma once
#include "../../engine/include/Order.hpp"
#include <string>

// Thin wrapper around Drogon's async DB client for the two tables we care
// about: orders and trades. All calls are fire-and-forget async (we don't
// block the matching path on disk I/O); failures are logged, not thrown,
// since a persistence hiccup shouldn't take down order matching.
namespace Persistence {

void saveOrder(const Order& order);
void updateOrderStatus(uint64_t orderId, OrderStatus status, uint64_t remainingQty);
void saveTrade(const Trade& trade);

} // namespace Persistence
