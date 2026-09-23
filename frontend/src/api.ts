import type { OrderBookSnapshot, PlaceOrderResponse, Side, OrderType, Trade } from "./types";

// In dev, Vite's proxy (vite.config.ts) forwards /api to localhost:8080.
// In prod, VITE_API_URL is baked in at build time to point at the deployed
// backend (e.g. https://order-matching-backend.onrender.com).
const API_BASE = import.meta.env.VITE_API_URL ?? "";

export async function placeOrder(params: {
  symbol: string;
  side: Side;
  type: OrderType;
  price?: number;
  quantity: number;
}): Promise<PlaceOrderResponse> {
  const res = await fetch(`${API_BASE}/api/orders`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(params),
  });
  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: res.statusText }));
    throw new Error(err.error ?? `Request failed with ${res.status}`);
  }
  return res.json();
}

export async function cancelOrder(symbol: string, orderId: number): Promise<void> {
  const res = await fetch(`${API_BASE}/api/orders/${symbol}/${orderId}`, {
    method: "DELETE",
  });
  if (!res.ok) {
    const err = await res.json().catch(() => ({ error: res.statusText }));
    throw new Error(err.error ?? `Request failed with ${res.status}`);
  }
}

export async function getOrderBook(symbol: string, depth = 10): Promise<OrderBookSnapshot> {
  const res = await fetch(`${API_BASE}/api/orderbook/${symbol}?depth=${depth}`);
  if (!res.ok) throw new Error(`Failed to fetch order book: ${res.status}`);
  return res.json();
}

export async function getTrades(symbol: string): Promise<Trade[]> {
  const res = await fetch(`${API_BASE}/api/trades/${symbol}`);
  if (!res.ok) throw new Error(`Failed to fetch trades: ${res.status}`);
  const data = await res.json();
  return data.trades;
}
