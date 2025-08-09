"""
ws_feeder.py
------------
Author: Nick Ingargiola

Purpose:
    Streams live Binance book ticker data over WebSocket and generates synthetic
    BUY/SELL orders at a configurable rate. Designed to feed into the C++ Limit
    Order Book engine for real-time integration tests or performance demos.

    This script connects to Binance.US's public WebSocket endpoint for a given
    trading pair (default: BTC/USDT), listens for best bid/ask updates, and
    outputs randomized buy/sell orders around the mid-price.

Typical usage:
    python3 ws_feeder.py --symbol btcusdt --rate 0.15 --burst 3 --duration 5

Key parameters:
    --symbol   : Trading pair to stream (lowercased in URL).
    --rate     : Seconds between bursts of synthetic orders.
    --burst    : Number of orders emitted per incoming price update.
    --limit    : Stop after sending N orders (overrides duration if earlier).
    --duration : Stop after running for N seconds.
    --seed     : Optional RNG seed for reproducibility.

Output format (stdout):
    Each order line: "<SIDE> <PRICE> <QUANTITY>"
    At completion:   "EXIT" (signals downstream engine to stop reading)
"""

import asyncio
import json
import random
import sys
import argparse
import time
import websockets

# Default Binance.US book ticker endpoint for BTC/USDT
STREAM = "wss://stream.binance.us:9443/ws/btcusdt@bookTicker"


async def run(symbol="btcusdt", rate=0.15, burst=3, limit=None, duration=None, seed=None):
    """
    Connect to Binance WebSocket stream for `symbol` and emit synthetic orders.

    Args:
        symbol (str)      : Market symbol (e.g., "btcusdt") to subscribe to.
        rate (float)      : Seconds to wait between each synthetic order burst.
        burst (int)       : Orders generated per price update.
        limit (int|None)  : Stop after sending this many total orders.
        duration (float|None): Stop after running this many seconds.
        seed (int|None)   : RNG seed for reproducibility.

    Behavior:
        - Subscribes to the Binance bookTicker stream.
        - For each incoming message, extracts best bid/ask prices.
        - Computes the mid-price and applies a small random "jitter" to simulate market variation.
        - Randomly selects BUY or SELL and outputs price/quantity to stdout.
        - Stops when `limit` or `duration` is reached.
    """
    if seed is not None:
        random.seed(seed)

    url = f"wss://stream.binance.us:9443/ws/{symbol.lower()}@bookTicker"
    start = time.time()
    sent = 0

    # Connect to Binance WebSocket
    async with websockets.connect(url, ping_interval=20, ping_timeout=20) as ws:
        while True:
            # Check stop conditions
            if duration and (time.time() - start) >= duration:
                break
            if limit and sent >= limit:
                break

            # Wait for next bid/ask update
            msg = await ws.recv()
            data = json.loads(msg)

            bid = float(data["b"])  # Best bid price
            ask = float(data["a"])  # Best ask price
            mid = (bid + ask) / 2   # Mid-market price

            # Emit a burst of synthetic orders
            for _ in range(burst):
                side = "BUY" if random.random() < 0.5 else "SELL"

                # Apply ±8 bps jitter to mid-price to simulate market micro-moves
                jitter_bps = random.uniform(-8, 8)
                price = round(mid * (1 + jitter_bps / 10000.0), 2)

                qty = random.randint(1, 5)  # Random small lot size
                sys.stdout.write(f"{side} {price} {qty}\n")
                sent += 1

            sys.stdout.flush()

            # Delay before next burst
            await asyncio.sleep(rate)

    # Tell downstream process to stop reading
    sys.stdout.write("EXIT\n")
    try:
        sys.stdout.flush()
    except BrokenPipeError:
        # Ignore if consumer process has already closed
        pass


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--symbol", default="btcusdt")
    p.add_argument("--rate", type=float, default=0.15, help="Seconds between bursts")
    p.add_argument("--burst", type=int, default=3, help="Orders per tick")
    p.add_argument("--limit", type=int, help="Stop after N orders")
    p.add_argument("--duration", type=float, help="Stop after N seconds")
    p.add_argument("--seed", type=int, help="Deterministic randomness")
    args = p.parse_args()

    try:
        asyncio.run(run(args.symbol, args.rate, args.burst, args.limit, args.duration, args.seed))
    except BrokenPipeError:
        # Happens when stdout consumer closes early — safe to ignore
        pass
