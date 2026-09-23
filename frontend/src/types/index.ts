export type Side = "BUY" | "SELL";
export type OrderType = "LIMIT" | "MARKET";
export type OrderStatus = "NEW" | "PARTIALLY_FILLED" | "FILLED" | "CANCELLED";

export interface OrderResponse {
  id: number;
  symbol: string;
  side: Side;
  type: OrderType;
  price: number;
  quantity: number;
  original_quantity: number;
  status: OrderStatus;
  timestamp: number;
}

export interface Trade {
  trade_id: number;
  buy_order_id: number;
  sell_order_id: number;
  symbol: string;
  price: number;
  quantity: number;
  timestamp: number;
}

export interface BookLevel {
  price: number;
  total_quantity: number;
  order_count: number;
}

export interface OrderBookSnapshot {
  symbol: string;
  bids: BookLevel[];
  asks: BookLevel[];
}

export interface PlaceOrderResponse {
  order: OrderResponse;
  trades: Trade[];
}

// Messages pushed over the WebSocket feed
export type WsMessage =
  | ({ channel: "orderbook" } & OrderBookSnapshot)
  | { channel: "trades"; symbol: string; trades: Trade[] };
