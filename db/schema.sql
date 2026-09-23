-- Order Matching Engine -- PostgreSQL schema
-- Run automatically against the `orders_db` database defined in docker-compose.yml
-- (mounted into the postgres container's /docker-entrypoint-initdb.d/).

CREATE TABLE IF NOT EXISTS orders (
    id                BIGINT PRIMARY KEY,
    symbol            VARCHAR(16)     NOT NULL,
    side              VARCHAR(4)      NOT NULL CHECK (side IN ('BUY', 'SELL')),
    type              VARCHAR(8)      NOT NULL CHECK (type IN ('LIMIT', 'MARKET')),
    price             DOUBLE PRECISION NOT NULL DEFAULT 0,
    quantity          BIGINT          NOT NULL,          -- remaining quantity, updated as fills happen
    original_quantity BIGINT          NOT NULL,
    status            VARCHAR(20)     NOT NULL DEFAULT 'NEW'
                          CHECK (status IN ('NEW', 'PARTIALLY_FILLED', 'FILLED', 'CANCELLED')),
    created_at        TIMESTAMPTZ     NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_orders_symbol_status ON orders (symbol, status);

CREATE TABLE IF NOT EXISTS trades (
    trade_id       BIGINT PRIMARY KEY,
    buy_order_id   BIGINT          NOT NULL,
    sell_order_id  BIGINT          NOT NULL,
    symbol         VARCHAR(16)     NOT NULL,
    price          DOUBLE PRECISION NOT NULL,
    quantity       BIGINT          NOT NULL,
    executed_at    TIMESTAMPTZ     NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_trades_symbol_time ON trades (symbol, executed_at DESC);
