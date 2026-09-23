#pragma once
#include <drogon/WebSocketController.h>

// Single WebSocket endpoint: ws://host/ws/market?symbol=AAPL
// (Drogon's WebSocketController doesn't bind path placeholders to handler
// arguments the way HttpController does, so the symbol is passed as a query
// parameter and read manually in handleNewConnection.)
//
// On connect, the client is subscribed to that symbol's book/trade feed and
// immediately sent a full snapshot; after that it receives push updates
// whenever BookManager::broadcastBookUpdate/broadcastTrades fires.
class MarketDataWs : public drogon::WebSocketController<MarketDataWs> {
public:
    void handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
                           std::string&& message,
                           const drogon::WebSocketMessageType& type) override;

    void handleNewConnection(const drogon::HttpRequestPtr& req,
                              const drogon::WebSocketConnectionPtr& conn) override;

    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/market", drogon::Get);
    WS_PATH_LIST_END
};
