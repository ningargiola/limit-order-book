# Limit Order Book Matching Engine

This is my high-performance C++17 Limit Order Book implementation. The aim was to build something lean, solid, and fast — with order matching, modifications, cancellations, and trade tracking by price-time priority — while keeping it flexible for future upgrades.

---

## How to Build & Run

```bash
# Build
make debug        # symbols, no optimizations
make release      # optimized

# Run interactive CLI
make run          # builds release then ./lob

# Synthetic benchmark (BENCH command via main)
make bench ORDERS=2000000

# Stress harness (standalone, fastest path)
make stress-run                       # default count (2M)
make stress-run-custom ORDERS=5000000 # custom count

# Unit + integration tests (GoogleTest)
make test         # uses GTEST_DIR from Makefile (override with GTEST_DIR=/path)

# Live feed from Binance (WebSocket) -> engine
make feed
```

---

## Example Run

Interactive example:

```
BUY 100 10
SELL 99 5
SELL 99 5
SELL 102 10
BUY 102 5
MODIFY 4 8 101
CANCEL 5
PRINT
```

Expected output:

```
TRADE: 5 @ $99
TRADE: 5 @ $99
TRADE: 5 @ $102
Order modified.
Order not found.
Order Book:
BIDS:
ASKS:
Total Volume Traded: 15 units
```

---

## Features

* FIFO matching within price levels
* Partial fill support
* Add, cancel, and modify orders by ID
* Prints current order book
* Tracks total matched volume
* CSV export for trades and snapshots
* Benchmark mode for throughput
* Live Binance feed integration
* Unit and stress testing

---

## Benchmarks

**Synthetic Benchmark Mode:**

* **1M orders** → \~1.09M trades/sec
* **2M orders** (optimized build) → \~4.21M trades/sec
* **2M orders** (current config with logging/tests) → \~1.01M trades/sec

---

## Roadmap

* Per-order status tracking (active, filled, canceled)
* Persistent book storage
* Real-time frontend dashboard
* More realistic synthetic flow for stress testing
* Lock-free/parallelized engine experiments

---

## File Structure

```bash
├── include/
│   ├── order.h
│   ├── order_book.h
│   └── trade.h
├── src/
│   ├── main.cpp
│   ├── order.cpp
│   └── order_book.cpp
├── tests/
│   ├── test_order_book.cpp
│   └── stress.cpp
├── ws_feeder.py
├── Makefile
└── README.md
```

---

## Notes

You can tweak logic or run specific scenarios by editing commands in `main.cpp` or piping in a file. Use `bench` for raw speed tests and `feed` for live market integration checks.
