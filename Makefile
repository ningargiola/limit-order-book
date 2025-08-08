# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Source and object files
SRC = src/main.cpp src/order.cpp src/order_book.cpp
OBJ = $(SRC:.cpp=.o)

# Output binary name
TARGET = xsh

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean target
clean:
	rm -f $(TARGET) src/*.o
