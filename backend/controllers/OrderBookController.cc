#include "OrderBookController.h"
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

void OrderBookController::getSnapshot(const HttpRequestPtr& req,
                                       std::function<void(const HttpResponsePtr&)>&& callback,
                                       std::string symbol) {
    symbol = toUpper(symbol);

    int depth = 10;
    auto depthParam = req->getParameter("depth");
    if (!depthParam.empty()) {
        try {
            depth = std::stoi(depthParam);
            depth = std::clamp(depth, 1, 50);
        } catch (...) {
            depth = 10;
        }
    }

    auto snap = BookManager::instance().getSnapshot(symbol, depth);
    auto resp = HttpResponse::newHttpJsonResponse(Json2::snapshotToJson(snap));
    callback(resp);
}
