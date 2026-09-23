#include "OrdersController.h"
#include "../services/BookManager.h"
#include "../services/JsonSerializers.h"
#include <algorithm>
#include <cctype>

using namespace drogon;

namespace {

HttpResponsePtr errorResponse(HttpStatusCode code, const std::string& message) {
    Json::Value j;
    j["error"] = message;
    auto resp = HttpResponse::newHttpJsonResponse(j);
    resp->setStatusCode(code);
    return resp;
}

std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}

} // namespace

void OrdersController::createOrder(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback) {
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        callback(errorResponse(k400BadRequest, "Request body must be valid JSON"));
        return;
    }
    const Json::Value& body = *jsonPtr;

    // --- Validate required fields ---
    if (!body.isMember("symbol") || !body["symbol"].isString() || body["symbol"].asString().empty()) {
        callback(errorResponse(k400BadRequest, "'symbol' is required"));
        return;
    }
    if (!body.isMember("side") || !body["side"].isString()) {
        callback(errorResponse(k400BadRequest, "'side' is required (BUY or SELL)"));
        return;
    }
    bool quantityIsNumeric = body.isMember("quantity") &&
                              (body["quantity"].isUInt64() || body["quantity"].isInt());
    if (!quantityIsNumeric) {
        callback(errorResponse(k400BadRequest, "'quantity' is required and must be a positive integer"));
        return;
    }

    std::string symbol = toUpper(body["symbol"].asString());
    std::string sideStr = toUpper(body["side"].asString());
    std::string typeStr = body.get("type", "LIMIT").asString();
    typeStr = toUpper(typeStr);

    if (sideStr != "BUY" && sideStr != "SELL") {
        callback(errorResponse(k400BadRequest, "'side' must be BUY or SELL"));
        return;
    }
    if (typeStr != "LIMIT" && typeStr != "MARKET") {
        callback(errorResponse(k400BadRequest, "'type' must be LIMIT or MARKET"));
        return;
    }

    Side side = (sideStr == "BUY") ? Side::BUY : Side::SELL;
    OrderType type = (typeStr == "LIMIT") ? OrderType::LIMIT : OrderType::MARKET;

    int64_t quantity = body["quantity"].asInt64();
    if (quantity <= 0) {
        callback(errorResponse(k400BadRequest, "'quantity' must be greater than 0"));
        return;
    }

    double price = 0.0;
    if (type == OrderType::LIMIT) {
        if (!body.isMember("price") || !body["price"].isNumeric()) {
            callback(errorResponse(k400BadRequest, "'price' is required for LIMIT orders"));
            return;
        }
        price = body["price"].asDouble();
        if (price <= 0.0) {
            callback(errorResponse(k400BadRequest, "'price' must be greater than 0"));
            return;
        }
    }

    auto result = BookManager::instance().placeOrder(symbol, side, type, price,
                                                       static_cast<uint64_t>(quantity));

    Json::Value response;
    response["order"] = Json2::orderToJson(result.order);
    Json::Value tradesJson(Json::arrayValue);
    for (const auto& t : result.trades) tradesJson.append(Json2::tradeToJson(t));
    response["trades"] = tradesJson;

    auto resp = HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(k201Created);
    callback(resp);
}

void OrdersController::cancelOrder(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string symbol,
                                    uint64_t orderId) {
    symbol = toUpper(symbol);
    bool cancelled = BookManager::instance().cancelOrder(symbol, orderId);

    if (!cancelled) {
        callback(errorResponse(k404NotFound, "Order not found or already filled/cancelled"));
        return;
    }

    Json::Value j;
    j["cancelled"] = true;
    j["order_id"] = static_cast<Json::UInt64>(orderId);
    callback(HttpResponse::newHttpJsonResponse(j));
}
