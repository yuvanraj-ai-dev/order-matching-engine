#pragma once
#include <drogon/HttpController.h>

class OrderBookController : public drogon::HttpController<OrderBookController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(OrderBookController::getSnapshot, "/api/orderbook/{1}", drogon::Get);
    METHOD_LIST_END

    // depth is read from the optional ?depth= query parameter inside the
    // handler (defaults to 10) rather than being bound as a path segment,
    // to keep routing unambiguous.
    void getSnapshot(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      std::string symbol);
};
