import { useEffect, useRef, useState } from "react";
import type { OrderBookSnapshot, Trade, WsMessage } from "../types";

// If VITE_WS_URL is explicitly set, use it.
// Otherwise, try to infer it from VITE_API_URL (replace http/https with ws/wss).
// Fallback to the current host (using wss: if the page is https:).
let WS_BASE = import.meta.env.VITE_WS_URL;
if (!WS_BASE) {
  const apiBase = import.meta.env.VITE_API_URL;
  if (apiBase) {
    WS_BASE = apiBase.replace(/^http/, "ws");
  } else {
    const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
    WS_BASE = `${protocol}//${window.location.host}`;
  }
}

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
      try {
        socket = new WebSocket(`${WS_BASE}/ws/market?symbol=${symbol}`);
      } catch (err) {
        console.error("Failed to construct WebSocket:", err);
        // Retry later using the same backoff
        setTimeout(connect, retryDelay);
        retryDelay = Math.min(retryDelay * 2, 15000);
        return;
      }
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
