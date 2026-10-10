# Real-Time Order Matching Engine

A high-performance limit order matching engine featuring a C++ core, a Drogon (C++) REST and WebSocket API, a React/TypeScript trading dashboard, and PostgreSQL/Redis for persistence and real-time fan-out.

**Stack:** C++17, Drogon, React, TypeScript, PostgreSQL, Redis, WebSockets, Docker

---

## Architecture

```text
┌─────────────┐        ┌──────────────────────────────┐
│   React     │◄──────►│         Drogon (C++)          │
│  Frontend   │ REST/  │  ┌─────────────┐  ┌─────────┐ │
│  (nginx)    │  WS    │  │ OrderBook   │  │ Drogon  │ │
│             │        │  │ (matching   │  │ REST/WS │ │
│             │        │  │  engine)    │  │ layer   │ │
│             │        │  └─────────────┘  └─────────┘ │
└─────────────┘        └───────┬──────────────┬────────┘
                                │              │
                       ┌────────┴───┐   ┌──────┴──────┐
                       │ PostgreSQL │   │    Redis    │
                       │ (orders,   │   │  (pub/sub   │
                       │  trades)   │   │  fan-out)   │
                       └────────────┘   └─────────────┘
```

The core matching engine is a standalone, dependency-free C++ static library. The backend links it directly and wraps it with a REST and WebSocket API. 

## Features

- **Price-Time Priority Matching**: Incoming orders match against the best price first; orders at the same price fill in FIFO order.
- **Low Latency Engine**: Engineered for high throughput using `std::map` (O(log n) insert/cancel).
- **Real-Time Market Data**: In-process WebSocket broadcast for live updates, with a parallel Redis PUBLISH for horizontal scaling.
- **Async Persistence**: Orders and trades are asynchronously written to PostgreSQL.

---

## Quickstart

Run the entire stack locally using Docker Compose:

```bash
docker compose up --build
```

- **Frontend:** http://localhost:3000
- **Backend API:** http://localhost:8080

*(The PostgreSQL database will automatically initialize schemas on first startup).*

---

## API Reference

| Method | Path | Description |
|---|---|---|
| GET | `/health` | Health check |
| POST | `/api/orders` | Place an order — body: `{symbol, side, type, price?, quantity}` |
| DELETE | `/api/orders/{symbol}/{id}` | Cancel a resting order |
| GET | `/api/orderbook/{symbol}?depth=10` | Current book snapshot (top N levels) |
| GET | `/api/trades/{symbol}` | Last 100 trades |
| WS | `/ws/market?symbol=AAPL` | Live push feed: full snapshot on connect, then book/trade updates |

**Example Order:**
```bash
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{"symbol": "AAPL", "side": "BUY", "type": "LIMIT", "price": 100.50, "quantity": 10}'
```

---

## License

MIT License
