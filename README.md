# Real-Time Order Matching Engine

A limit order matching engine (like a mini stock exchange) with a C++ core, a
Drogon (C++) REST + WebSocket API, a React/TypeScript trading dashboard, and
PostgreSQL + Redis for persistence and live fan-out.

**Stack:** C++17, Drogon, React, TypeScript, PostgreSQL, Redis, WebSockets, Docker

---

## Architecture

```
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

The matching engine (`engine/`) is a standalone, dependency-free C++ static
library — it doesn't know about HTTP, JSON, or Drogon at all. The backend
(`backend/`) links it directly and wraps it with a REST + WebSocket API. This
separation means the engine can be unit-tested and benchmarked in complete
isolation from the web layer (see `engine/tests` and `engine/benchmark`).

## Why this design

- **Price-time priority matching**: incoming orders match against the best
  price first; orders at the same price fill in FIFO order.
- **Order book = `std::map` of price → FIFO list**, giving O(log n)
  insert/match and O(1) cancellation via a side `unordered_map<orderId,
  iterator>` lookup. See the "Data structure choice" note below.
- **In-process WebSocket broadcast** for pushing live updates to connected
  browsers (fast, no extra hop), with a **parallel Redis PUBLISH** on every
  book/trade update as a horizontal-scaling hook — if this ran as multiple
  backend instances behind a load balancer, each could subscribe to Redis to
  stay in sync with trades matched on a different instance.
- **Fire-and-forget async persistence**: every order and trade is written to
  Postgres asynchronously so a slow disk/DB never blocks the matching path.

### Data structure choice: why `std::map` (red-black tree) over a priority queue

A plain priority queue gives fast access to the best price but can't
efficiently cancel or modify an order in the middle of the book (O(n) search).
Real exchanges cancel/replace orders constantly, so that's a dealbreaker.
`std::map<price, list<Order>>` (a red-black tree under the hood) gives
O(log n) insert/cancel, O(1) access to the best price via `begin()`, and
natural in-order traversal for market depth — at the cost of being slower
than the price-indexed-array approach real exchanges use once you're
optimizing for microseconds instead of milliseconds (see Benchmarks below for
where this project sits on that spectrum, and what a further optimization
pass would look like).

---

## Benchmarks

Measured with `engine/benchmark/benchmark.cpp` — 1,000,000 randomly generated
limit orders, single-threaded, `-O2`/`-O3`, no I/O in the timed loop:

| Run | Orders | Time | Throughput |
|---|---|---|---|
| This sandbox (dev container) | 1,000,000 | 0.86s | **~1.16M orders/sec** |
| This sandbox (dev container) | 500,000 | 0.17s | **~3.0M orders/sec** |

**Re-run it yourself and report your own number** — throughput is CPU- and
compiler-dependent, and an interviewer may ask how you measured it:

```bash
cd engine
g++ -std=c++17 -O2 -Iinclude -o benchmark src/OrderBook.cpp benchmark/benchmark.cpp
./benchmark 1000000
```

For context: production exchange matching engines run in the low millions to
tens of millions of orders/sec using custom allocators, lock-free structures,
and price-indexed arrays instead of trees — worth knowing as a talking point
on "how would you make this faster," even though it's out of scope here.

---

## Project layout

```
order-matching-engine/
├── engine/          # Standalone C++ matching engine (no external deps)
│   ├── include/     # Order.hpp, OrderBook.hpp
│   ├── src/         # OrderBook.cpp, main.cpp (CLI demo)
│   ├── tests/       # Self-contained unit tests (no test framework needed)
│   ├── benchmark/   # Throughput benchmark harness
│   └── CMakeLists.txt
├── backend/         # Drogon C++ REST + WebSocket API
│   ├── controllers/ # HTTP route handlers
│   ├── ws/          # WebSocket controller (live market data feed)
│   ├── services/    # BookManager, Persistence (Postgres), RedisPublisher
│   ├── main.cc
│   ├── config.json  # DB/Redis connection config (env-var driven)
│   ├── CMakeLists.txt
│   └── Dockerfile
├── frontend/        # React + TypeScript trading dashboard
│   ├── src/
│   │   ├── components/  # OrderForm, OrderBookView, TradeTape, PriceChart
│   │   ├── hooks/        # useMarketData (WebSocket hook)
│   │   └── api.ts        # REST client
│   ├── Dockerfile
│   └── nginx.conf
├── db/
│   └── schema.sql   # orders + trades tables
├── docker-compose.yml
├── .env.example
└── README.md
```

---

## Running it locally

### Option A — Docker Compose (recommended, spins up everything)

```bash
docker compose up --build
```

- Frontend: http://localhost:3000
- Backend API: http://localhost:8080
- Postgres: localhost:5432 (user `orders_user` / db `orders_db`, see `docker-compose.yml`)
- Redis: localhost:6379

The Postgres container automatically runs `db/schema.sql` on first startup.

### Option B — Run pieces individually (faster iteration while developing)

**Engine (no dependencies beyond g++):**
```bash
cd engine
g++ -std=c++17 -O2 -Wall -Iinclude -o run_tests src/OrderBook.cpp tests/test_orderbook.cpp
./run_tests
```

**Backend (needs Drogon installed — see backend/Dockerfile for the apt packages, or `apt install libdrogon-dev` on Ubuntu 24.04+):**
```bash
cd backend
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
DB_HOST=localhost DB_NAME=orders_db DB_USER=orders_user DB_PASSWORD=orders_pass \
REDIS_HOST=localhost ./order_matching_backend
```
> If you don't have Postgres/Redis running locally, comment out the
> `db_clients`/`redis_clients` blocks in `backend/config.json` — the app is
> written to skip persistence/pub-sub gracefully (logs a warning, doesn't
> crash) if those clients aren't configured.

**Frontend:**
```bash
cd frontend
npm install
npm run dev
```
Vite's dev server proxies `/api` and `/ws` to `localhost:8080` (see `vite.config.ts`), so no CORS setup needed locally.

---

## API reference

| Method | Path | Description |
|---|---|---|
| GET | `/health` | Health check |
| POST | `/api/orders` | Place an order — body: `{symbol, side, type, price?, quantity}` |
| DELETE | `/api/orders/{symbol}/{id}` | Cancel a resting order |
| GET | `/api/orderbook/{symbol}?depth=10` | Current book snapshot (top N levels) |
| GET | `/api/trades/{symbol}` | Last 100 trades |
| WS | `/ws/market?symbol=AAPL` | Live push feed: full snapshot on connect, then book/trade updates |

Example order:
```bash
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{"symbol": "AAPL", "side": "BUY", "type": "LIMIT", "price": 100.50, "quantity": 10}'
```

---

## Testing

- **Unit tests** (`engine/tests/test_orderbook.cpp`): full fill, partial
  fill, no-match resting order, price-time priority FIFO, price priority
  across levels, cancel resting order, cancel-already-filled edge case,
  market order sweeping multiple levels, non-crossing prices. Run via
  `run_tests` (see above) — 29 assertions, all passing.
- **API testing**: hit the endpoints above with `curl`/Postman, or write a
  `pytest` + `httpx` script against the running container.
- **Load testing**: point [`wrk`](https://github.com/wg/wrk) or
  [`locust`](https://locust.io/) at `/api/orders` once the stack is running,
  to get a real end-to-end (not just in-memory) orders/sec number.

---

## Known limitations (good interview talking points)

- Single-process, single-node — no sharding of the order book across
  machines. The Redis pub/sub hook exists for this, but isn't load-bearing
  yet since there's only one backend instance.
- No authentication/authorization on order placement — anyone can place
  orders as anyone. A real system would tie orders to an authenticated
  account.
- No order types beyond LIMIT/MARKET (no stop orders, IOC/FOK, etc.).
- MARKET orders that can't be fully filled by resting liquidity are dropped
  rather than converted to a resting limit order — a deliberate
  simplification, noted in `OrderBook::addOrder`.

## License

MIT — do whatever you want with this for your own portfolio/resume.
