#pragma once
#include <drogon/HttpController.h>

class OrdersController : public drogon::HttpController<OrdersController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(OrdersController::createOrder, "/api/orders", drogon::Post);
    ADD_METHOD_TO(OrdersController::cancelOrder, "/api/orders/{1}/{2}", drogon::Delete);
    // {1} = symbol, {2} = order id -- cancellation needs the symbol to find the right book
    METHOD_LIST_END

    void createOrder(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void cancelOrder(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      std::string symbol,
                      uint64_t orderId);
};
