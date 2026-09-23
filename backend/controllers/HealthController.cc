#include "HealthController.h"

using namespace drogon;

void HealthController::check(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback) {
    Json::Value j;
    j["status"] = "ok";
    j["service"] = "order-matching-engine";
    auto resp = HttpResponse::newHttpJsonResponse(j);
    callback(resp);
}
