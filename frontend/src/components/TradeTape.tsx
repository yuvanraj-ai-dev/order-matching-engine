import type { Trade } from "../types";

function formatTime(nanos: number): string {
  const ms = nanos / 1e6;
  return new Date(ms).toLocaleTimeString();
}

export function TradeTape({ trades }: { trades: Trade[] }) {
  return (
    <div className="trade-tape">
      <h3>Recent Trades</h3>
      <div className="tape-header">
        <span>Time</span>
        <span>Price</span>
        <span>Qty</span>
      </div>
      <div className="tape-body">
        {trades.length === 0 && <div className="empty-row">No trades yet</div>}
        {trades.map((t) => (
          <div className="tape-row" key={t.trade_id}>
            <span className="time">{formatTime(t.timestamp)}</span>
            <span className="price">{t.price.toFixed(2)}</span>
            <span className="qty">{t.quantity}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
