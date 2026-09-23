import { useState, FormEvent } from "react";
import { placeOrder } from "../api";
import type { Side, OrderType } from "../types";

export function OrderForm({ symbol, onOrderPlaced }: { symbol: string; onOrderPlaced: () => void }) {
  const [side, setSide] = useState<Side>("BUY");
  const [type, setType] = useState<OrderType>("LIMIT");
  const [price, setPrice] = useState("100.00");
  const [quantity, setQuantity] = useState("10");
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [lastResult, setLastResult] = useState<string | null>(null);

  async function handleSubmit(e: FormEvent) {
    e.preventDefault();
    setError(null);
    setSubmitting(true);
    try {
      const result = await placeOrder({
        symbol,
        side,
        type,
        price: type === "LIMIT" ? parseFloat(price) : undefined,
        quantity: parseInt(quantity, 10),
      });
      const filled = result.trades.reduce((sum, t) => sum + t.quantity, 0);
      setLastResult(
        filled > 0
          ? `Order #${result.order.id}: ${filled} filled across ${result.trades.length} trade(s), status ${result.order.status}`
          : `Order #${result.order.id} resting on the book (no immediate match)`
      );
      onOrderPlaced();
    } catch (err) {
      setError(err instanceof Error ? err.message : "Failed to place order");
    } finally {
      setSubmitting(false);
    }
  }

  return (
    <form className="order-form" onSubmit={handleSubmit}>
      <h3>Place Order — {symbol}</h3>

      <div className="side-toggle">
        <button type="button" className={side === "BUY" ? "active buy" : "buy"} onClick={() => setSide("BUY")}>
          BUY
        </button>
        <button type="button" className={side === "SELL" ? "active sell" : "sell"} onClick={() => setSide("SELL")}>
          SELL
        </button>
      </div>

      <label>
        Order Type
        <select value={type} onChange={(e) => setType(e.target.value as OrderType)}>
          <option value="LIMIT">LIMIT</option>
          <option value="MARKET">MARKET</option>
        </select>
      </label>

      {type === "LIMIT" && (
        <label>
          Price
          <input
            type="number"
            step="0.01"
            min="0.01"
            value={price}
            onChange={(e) => setPrice(e.target.value)}
            required
          />
        </label>
      )}

      <label>
        Quantity
        <input
          type="number"
          step="1"
          min="1"
          value={quantity}
          onChange={(e) => setQuantity(e.target.value)}
          required
        />
      </label>

      <button type="submit" className={`submit-btn ${side.toLowerCase()}`} disabled={submitting}>
        {submitting ? "Placing…" : `Place ${side} Order`}
      </button>

      {error && <p className="error">{error}</p>}
      {lastResult && <p className="result">{lastResult}</p>}
    </form>
  );
}
