#include "RedisPublisher.h"
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

using namespace drogon;

namespace {
void publish(const std::string& channel, const std::string& payload) {
    auto redis = app().getRedisClient();
    if (!redis) return; // Redis not configured -- app still runs, just skips fan-out

    redis->execCommandAsync(
        [](const nosql::RedisResult&) { /* PUBLISH ack, nothing to do */ },
        [](const nosql::RedisException& e) {
            LOG_ERROR << "Redis publish failed: " << e.what();
        },
        "PUBLISH %s %s", channel.c_str(), payload.c_str());
}
}

namespace RedisPublisher {

void publishBookUpdate(const std::string& symbol, const std::string& jsonSnapshot) {
    publish("orderbook:" + symbol, jsonSnapshot);
}

void publishTrade(const std::string& symbol, const std::string& jsonTrade) {
    publish("trades:" + symbol, jsonTrade);
}

} // namespace RedisPublisher
