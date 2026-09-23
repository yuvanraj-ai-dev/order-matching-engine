import { useState } from "react";
import { useMarketData } from "./hooks/useMarketData";
import { OrderForm } from "./components/OrderForm";
import { OrderBookView } from "./components/OrderBookView";
import { TradeTape } from "./components/TradeTape";
import { PriceChart } from "./components/PriceChart";

const SYMBOLS = ["AAPL", "MSFT", "GOOGL", "TSLA"];

export default function App() {
  const [symbol, setSymbol] = useState("AAPL");
  const { book, trades, connected } = useMarketData(symbol);

  return (
    <div className="app">
      <header className="app-header">
        <h1>Real-Time Order Matching Engine</h1>
        <div className="header-right">
          <select value={symbol} onChange={(e) => setSymbol(e.target.value)}>
            {SYMBOLS.map((s) => (
              <option key={s} value={s}>
                {s}
              </option>
            ))}
          </select>
          <span className={`status-dot ${connected ? "connected" : "disconnected"}`} />
          <span className="status-label">{connected ? "Live" : "Reconnecting…"}</span>
        </div>
      </header>

      <main className="app-grid">
        <section className="col left">
          <OrderBookView book={book} />
        </section>

        <section className="col center">
          <PriceChart trades={trades} />
          <TradeTape trades={trades} />
        </section>

        <section className="col right">
          <OrderForm symbol={symbol} onOrderPlaced={() => {}} />
        </section>
      </main>
    </div>
  );
}
