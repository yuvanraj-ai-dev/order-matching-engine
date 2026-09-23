import type { OrderBookSnapshot } from "../types";

export function OrderBookView({ book }: { book: OrderBookSnapshot | null }) {
  const maxQty = book
    ? Math.max(
        1,
        ...book.bids.map((l) => l.total_quantity),
        ...book.asks.map((l) => l.total_quantity)
      )
    : 1;

  return (
    <div className="order-book">
      <h3>Order Book {book ? `— ${book.symbol}` : ""}</h3>
      <div className="book-columns">
        <div className="book-side asks">
          <div className="book-header">
            <span>Price</span>
            <span>Qty</span>
          </div>
          {book?.asks.length ? (
            [...book.asks].reverse().map((lvl) => (
              <div className="book-row" key={`ask-${lvl.price}`}>
                <div
                  className="depth-bar ask-bar"
                  style={{ width: `${(lvl.total_quantity / maxQty) * 100}%` }}
                />
                <span className="price ask-price">{lvl.price.toFixed(2)}</span>
                <span className="qty">{lvl.total_quantity}</span>
              </div>
            ))
          ) : (
            <div className="empty-row">No asks</div>
          )}
        </div>

        <div className="book-side bids">
          <div className="book-header">
            <span>Price</span>
            <span>Qty</span>
          </div>
          {book?.bids.length ? (
            book.bids.map((lvl) => (
              <div className="book-row" key={`bid-${lvl.price}`}>
                <div
                  className="depth-bar bid-bar"
                  style={{ width: `${(lvl.total_quantity / maxQty) * 100}%` }}
                />
                <span className="price bid-price">{lvl.price.toFixed(2)}</span>
                <span className="qty">{lvl.total_quantity}</span>
              </div>
            ))
          ) : (
            <div className="empty-row">No bids</div>
          )}
        </div>
      </div>
    </div>
  );
}
