#pragma once
#include <string>

// Publishes book/trade updates to Redis channels. This is separate from the
// in-process WebSocket broadcast in BookManager: the in-process broadcast is
// what actually pushes updates to connected browser clients (fast, no extra
// hop), while this Redis publish exists as a scale-out hook -- if you ever
// ran multiple backend instances behind a load balancer, each instance could
// subscribe to these channels to stay in sync with trades matched on a
// different instance. For a single-instance deployment it's optional, but
// it's what makes the architecture horizontally scalable later, and it's a
// reasonable thing to point to in an interview.
namespace RedisPublisher {

void publishBookUpdate(const std::string& symbol, const std::string& jsonSnapshot);
void publishTrade(const std::string& symbol, const std::string& jsonTrade);

} // namespace RedisPublisher
