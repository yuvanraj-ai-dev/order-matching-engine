import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid } from "recharts";
import type { Trade } from "../types";

export function PriceChart({ trades }: { trades: Trade[] }) {
  // Trades arrive newest-first; chart wants chronological order.
  const data = [...trades]
    .reverse()
    .map((t) => ({ time: new Date(t.timestamp / 1e6).toLocaleTimeString(), price: t.price }));

  return (
    <div className="price-chart">
      <h3>Price</h3>
      {data.length < 2 ? (
        <div className="empty-row">Waiting for enough trades to chart…</div>
      ) : (
        <ResponsiveContainer width="100%" height={220}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" stroke="#2a2f3a" />
            <XAxis dataKey="time" tick={{ fontSize: 10, fill: "#8892a0" }} minTickGap={30} />
            <YAxis domain={["auto", "auto"]} tick={{ fontSize: 10, fill: "#8892a0" }} width={55} />
            <Tooltip contentStyle={{ background: "#1a1e27", border: "1px solid #2a2f3a" }} />
            <Line type="monotone" dataKey="price" stroke="#4ade80" dot={false} strokeWidth={2} isAnimationActive={false} />
          </LineChart>
        </ResponsiveContainer>
      )}
    </div>
  );
}
