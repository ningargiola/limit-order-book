# ============================================================================
# Limit Order Book Makefile
# Author: Nick Ingargiola
# Purpose:
#   Build, test, and benchmark a high-performance C++17 Limit Order Book.
#   Includes:
#     - Debug and Release builds
#     - GoogleTest unit testing
#     - Stress testing
#     - Live feed integration
# ============================================================================

# ------------------------------
# Compiler settings
# ------------------------------
CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -Wall -Iinclude
LDFLAGS   ?=

# ------------------------------
# Source files
# ------------------------------
SRC        = src/main.cpp src/order.cpp src/order_book.cpp
TARGET     = lob

# ------------------------------
# GoogleTest settings
# ------------------------------
TEST_SRC     = tests/test_order_book.cpp src/order.cpp src/order_book.cpp
TEST_TARGET  = test_lob
GTEST_DIR   ?= /opt/homebrew/Cellar/googletest/1.17.0/
GTEST_FLAGS  = -I$(GTEST_DIR)/include -L$(GTEST_DIR)/lib -lgtest -lgtest_main -pthread

# ------------------------------
# Default build (debug)
# ------------------------------
all: debug

# Debug build: symbols, no optimizations
debug: CXXFLAGS += -g -O0
debug: $(TARGET)

# Release build: optimizations enabled
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# ------------------------------
# Link/build
# ------------------------------
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

# ------------------------------
# Benchmarks
# ------------------------------
# Usage: make bench ORDERS=100000
ORDERS ?= 100000
bench: release
	@echo "Running BENCH $(ORDERS)..."
	@printf "BENCH $(ORDERS)\nEXIT\n" | ./$(TARGET)

# ------------------------------
# Live data feed (requires Python)
# ------------------------------
feed: release
	@python3 ws_feeder.py | ./$(TARGET)

# ------------------------------
# Interactive run
# ------------------------------
run: release
	@./$(TARGET)

# ------------------------------
# Unit tests (GoogleTest)
# Usage: make test GTEST_DIR=/path/to/googletest/build
# ------------------------------
test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	@if [ -z "$(GTEST_DIR)" ]; then \
		echo "Error: Please set GTEST_DIR to your GoogleTest build directory"; \
		exit 1; \
	fi
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SRC) $(GTEST_FLAGS)

# ------------------------------
# Stress testing
# ------------------------------
# Build stress binary
stress: CXXFLAGS += -O3 -DNDEBUG
stress: TARGET_STRESS = stress
stress: tests/stress.cpp src/order.cpp src/order_book.cpp
	$(CXX) $(CXXFLAGS) -Iinclude -o $(TARGET_STRESS) tests/stress.cpp src/order.cpp src/order_book.cpp

# Run stress test with default 2M orders
stress-run: stress
	@./stress

# Run stress with custom order count
# Example: make stress-run-custom ORDERS=5000000
stress-run-custom: stress
	@./stress $(ORDERS)

# ------------------------------
# Cleanup
# ------------------------------
clean:
	rm -f $(TARGET) $(TEST_TARGET) stress exports/*

.PHONY: all debug release bench feed run clean test stress stress-run stress-run-custom
