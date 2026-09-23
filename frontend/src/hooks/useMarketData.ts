import { useEffect, useRef, useState } from "react";
import type { OrderBookSnapshot, Trade, WsMessage } from "../types";

const WS_BASE = import.meta.env.VITE_WS_URL ?? `ws://${window.location.host}`;

// Subscribes to a symbol's live feed and keeps the current order book
// snapshot + a rolling trade tape in React state. Reconnects automatically
// with backoff if the connection drops.
export function useMarketData(symbol: string) {
  const [book, setBook] = useState<OrderBookSnapshot | null>(null);
  const [trades, setTrades] = useState<Trade[]>([]);
  const [connected, setConnected] = useState(false);
  const wsRef = useRef<WebSocket | null>(null);

  useEffect(() => {
    let cancelled = false;
    let retryDelay = 1000;
    let socket: WebSocket;

    function connect() {
      socket = new WebSocket(`${WS_BASE}/ws/market?symbol=${symbol}`);
      wsRef.current = socket;

      socket.onopen = () => {
        if (cancelled) return;
        setConnected(true);
        retryDelay = 1000; // reset backoff on a healthy connection
      };

      socket.onmessage = (event) => {
        if (cancelled) return;
        try {
          const msg: WsMessage = JSON.parse(event.data);
          if (msg.channel === "orderbook") {
            setBook({ symbol: msg.symbol, bids: msg.bids, asks: msg.asks });
          } else if (msg.channel === "trades") {
            setTrades((prev) => [...msg.trades, ...prev].slice(0, 50));
          }
        } catch {
          // ignore malformed frames (e.g. a stray "pong")
        }
      };

      socket.onclose = () => {
        if (cancelled) return;
        setConnected(false);
        setTimeout(connect, retryDelay);
        retryDelay = Math.min(retryDelay * 2, 15000);
      };

      socket.onerror = () => {
        socket.close();
      };
    }

    connect();

    return () => {
      cancelled = true;
      wsRef.current?.close();
    };
  }, [symbol]);

  return { book, trades, connected };
}
