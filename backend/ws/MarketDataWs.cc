#include "MarketDataWs.h"
#include "../services/BookManager.h"
#include "../services/JsonSerializers.h"
#include <trantor/utils/Logger.h>
#include <algorithm>
#include <cctype>

using namespace drogon;

namespace {
std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
    return s;
}
}

void MarketDataWs::handleNewConnection(const HttpRequestPtr& req,
                                        const WebSocketConnectionPtr& conn) {
    std::string symbol = req->getParameter("symbol");
    if (symbol.empty()) symbol = "AAPL"; // sensible default for quick testing
    symbol = toUpper(symbol);

    // Stash the symbol on the connection's context so we can reuse it if we
    // ever need it outside BookManager's own reverse index.
    conn->setContext(std::make_shared<std::string>(symbol));

    BookManager::instance().subscribe(symbol, conn);

    // Send an immediate full snapshot so the client doesn't have to wait for
    // the next trade to see the current book state.
    auto snap = BookManager::instance().getSnapshot(symbol, 10);
    Json::Value j = Json2::snapshotToJson(snap);
    j["channel"] = "orderbook";
    conn->send(Json2::toCompactString(j));

    LOG_INFO << "WS client subscribed to " << symbol;
}

void MarketDataWs::handleConnectionClosed(const WebSocketConnectionPtr& conn) {
    BookManager::instance().unsubscribe(conn);
}

void MarketDataWs::handleNewMessage(const WebSocketConnectionPtr& conn,
                                     std::string&& message,
                                     const WebSocketMessageType& type) {
    // This feed is currently one-way (server -> client push on every trade /
    // book change). We don't expect inbound messages, but respond to pings
    // so the connection is kept alive by clients that ping over the data
    // channel instead of using WS-level ping frames.
    if (type == WebSocketMessageType::Text && message == "ping") {
        conn->send("pong");
    }
}
