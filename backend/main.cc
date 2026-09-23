#include <drogon/drogon.h>
#include <cstdlib>

using namespace drogon;

int main() {
    // Render (and most PaaS hosts) inject the port to bind via $PORT.
    // Fall back to 8080 for local/dev runs.
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::atoi(portEnv) : 8080;

    // If config.json is present next to the binary, Drogon will pick up
    // DB/Redis client config from there. We still explicitly set the
    // listener here so the port-from-env logic above always wins.
    app().addListener("0.0.0.0", port);

    app().setThreadNum(std::thread::hardware_concurrency());

    // Allow the React frontend (served from a different origin in dev, and
    // possibly a different Render service in prod) to call the API.
    app().registerPostHandlingAdvice(
        [](const HttpRequestPtr& req, const HttpResponsePtr& resp) {
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        });

    LOG_INFO << "Order Matching Engine backend starting on port " << port;

    app().run();
    return 0;
}
