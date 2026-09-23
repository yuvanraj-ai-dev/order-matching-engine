#pragma once
#include <drogon/HttpController.h>

class TradesController : public drogon::HttpController<TradesController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(TradesController::getTrades, "/api/trades/{1}", drogon::Get);
    METHOD_LIST_END

    void getTrades(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    std::string symbol);
};
