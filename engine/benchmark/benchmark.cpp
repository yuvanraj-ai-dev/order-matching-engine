// Throughput benchmark: feeds N randomly generated orders into the engine
// with no I/O in the hot path, and reports orders/sec.
//
// Usage: ./benchmark [num_orders]   (default 1,000,000)
#include "../include/OrderBook.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <vector>

int main(int argc, char** argv) {
    uint64_t N = (argc > 1) ? std::stoull(argv[1]) : 1'000'000;

    OrderBook book("AAPL");

    std::mt19937 rng(42); // fixed seed -> reproducible benchmark
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_real_distribution<double> priceDist(95.0, 105.0); // tight range -> lots of crossing
    std::uniform_int_distribution<int> qtyDist(1, 100);

    // Pre-generate orders so random number generation isn't counted in the timing.
    std::vector<Order> orders;
    orders.reserve(N);
    for (uint64_t i = 0; i < N; i++) {
        Side side = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
        double price = std::round(priceDist(rng) * 100.0) / 100.0;
        uint64_t qty = qtyDist(rng);
        orders.emplace_back(i + 1, "AAPL", side, OrderType::LIMIT, price, qty,
                             static_cast<int64_t>(i));
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (auto& o : orders) {
        book.addOrder(o);
    }
    auto end = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(end - start).count();
    double ordersPerSec = static_cast<double>(N) / seconds;

    std::cout << "Orders processed:    " << N << "\n";
    std::cout << "Time taken:          " << seconds << " s\n";
    std::cout << "Throughput:          " << static_cast<uint64_t>(ordersPerSec) << " orders/sec\n";
    std::cout << "Trades generated:    " << book.getTradeHistory().size() << "\n";
    std::cout << "Resting orders left: " << book.totalRestingOrders() << "\n";

    return 0;
}
