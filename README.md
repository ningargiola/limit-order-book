# Limit Order Book Matching Engine

This is a lightweight C++ project that implements a basic limit order book. The goal was to build something fast and extensible that gets the core functionality right: matching, canceling, modifying, and tracking volume. It's still early, but this covers the essentials.

---

## How to Build & Run

```bash
make
./lob
```

---

## What It Does (Current Test Run)

It runs a short hardcoded simulation (see `main.cpp`) to verify basic behaviors:

```text
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

## Current Features

* Price-time priority matching (FIFO per price level)
* Partial fills handled correctly
* Order insertion, cancelation, and modification by ID
* Prints current order book by price
* Tracks total matched volume

---

## Roadmap

* Trade history log (ID, price, qty, timestamp)
* Status tracking per order (e.g., filled, active, canceled)
* Performance testing: how fast can it match a million orders?
* Persistent book saving/loading
* Ability to feed in commands from a file
* Hook up a basic frontend to view book in real time
* Export to CSV or JSON for analysis
* Eventually rewrite internals to avoid STL and heap allocations
* Parallel or lock-free version for faster throughput
* Simulate realistic order flow using simple AI behaviors
* Optional AI-driven volume prediction (purely experimental)

---

## File Structure

```bash
├── include/
│   ├── order.h
│   └── order_book.h
├── src/
│   ├── main.cpp
│   ├── order.cpp
│   └── order_book.cpp
├── Makefile
└── README.md
```

---

## Notes

If you're testing or messing with the logic, just edit the `commands` list inside `main.cpp`. You can also adapt it to read from a file for larger runs.

---
