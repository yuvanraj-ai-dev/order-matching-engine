#include "TradesController.h"
#include "../services/BookManager.h"
#include "../services/JsonSerializers.h"
#include <algorithm>
#include <cctype>

using namespace drogon;

namespace {
std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}
}

void TradesController::getTrades(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  std::string symbol) {
    symbol = toUpper(symbol);
    auto trades = BookManager::instance().getTradeHistory(symbol);

    // Most recent first, capped to last 100 so the payload stays small.
    int limit = 100;
    Json::Value arr(Json::arrayValue);
    int count = 0;
    for (auto it = trades.rbegin(); it != trades.rend() && count < limit; ++it, ++count) {
        arr.append(Json2::tradeToJson(*it));
    }

    Json::Value j;
    j["symbol"] = symbol;
    j["trades"] = arr;
    callback(HttpResponse::newHttpJsonResponse(j));
}
