#pragma once
#include <drogon/HttpController.h>

class HealthController : public drogon::HttpController<HealthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::check, "/health", drogon::Get);
    METHOD_LIST_END

    void check(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
