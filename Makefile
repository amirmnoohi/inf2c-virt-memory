# ============================================================================
# Makefile for Virtual Memory and Cache Simulator
# ============================================================================

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude
DEBUG_FLAGS = -g -O0 -DDEBUG

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TEST_DIR = tests

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/config.c \
       $(SRC_DIR)/ll.c \
       $(SRC_DIR)/cache.c \
       $(SRC_DIR)/multilevel_cache.c \
       $(SRC_DIR)/tlb.c \
       $(SRC_DIR)/pagetable.c

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Output binary
TARGET = sim

# Default target
all: $(TARGET)

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# ============================================================================
# Test targets for each task
# ============================================================================

# Task 1: Fully associative cache
test-task1: $(TARGET)
	@echo "=== Testing Task 1: Fully Associative Cache ==="
	./$(TARGET) -S 1024 -T 4 -L 1 -t $(TEST_DIR)/task1/trace.txt

test-task1-v: $(TARGET)
	@echo "=== Testing Task 1 (Verbose) ==="
	./$(TARGET) -S 1024 -T 4 -L 1 -t $(TEST_DIR)/task1/trace.txt -v

# Task 2: Variable block sizes
test-task2: $(TARGET)
	@echo "=== Testing Task 2: Variable Block Sizes ==="
	./$(TARGET) -S 1024 -B 32 -T 4 -L 1 -t $(TEST_DIR)/task2/trace.txt

test-task2-v: $(TARGET)
	@echo "=== Testing Task 2 (Verbose) ==="
	./$(TARGET) -S 1024 -B 32 -T 4 -L 1 -t $(TEST_DIR)/task2/trace.txt -v

# Task 3: All associativities
test-task3: $(TARGET)
	@echo "=== Testing Task 3: Set-Associative Caches ==="
	./$(TARGET) -S 1024 -B 16 -A 3 -T 4 -L 1 -t $(TEST_DIR)/task3/trace.txt

test-task3-v: $(TARGET)
	@echo "=== Testing Task 3 (Verbose) ==="
	./$(TARGET) -S 1024 -B 16 -A 3 -T 4 -L 1 -t $(TEST_DIR)/task3/trace.txt -v

# Task 4: Multi-level cache
test-task4: $(TARGET)
	@echo "=== Testing Task 4: Multi-Level Cache ==="
	./$(TARGET) -S1 32768 -B1 64 -A1 2 -S2 262144 -B2 64 -A2 1 \
	            -T 4 -L 1 -t $(TEST_DIR)/task4/trace.txt

test-task4-v: $(TARGET)
	@echo "=== Testing Task 4 (Verbose) ==="
	./$(TARGET) -S1 32768 -B1 64 -A1 2 -S2 262144 -B2 64 -A2 1 \
	            -T 4 -L 1 -t $(TEST_DIR)/task4/trace.txt -v

# Run all tests with comparison
test: $(TARGET)
	@python3 tools/run_tests.py

# Run all tests (verbose, no comparison)
test-all: test-task1 test-task2 test-task3 test-task4

# ============================================================================
# Utility targets
# ============================================================================

# Check for compilation warnings
check: CFLAGS += -Wextra -Wpedantic
check: clean all

# Generate dependencies
depend:
	$(CC) -MM $(SRCS) > .depend

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build the simulator (default)"
	@echo "  debug        - Build with debug symbols"
	@echo "  clean        - Remove build artifacts"
	@echo "  test         - Run all tests with output comparison (colored)"
	@echo "  test-task1   - Test Task 1 (fully-associative)"
	@echo "  test-task2   - Test Task 2 (variable block sizes)"
	@echo "  test-task3   - Test Task 3 (all associativities)"
	@echo "  test-task4   - Test Task 4 (multi-level cache)"
	@echo "  test-all     - Run all tests (verbose, no comparison)"
	@echo "  check        - Build with extra warnings"
	@echo "  help         - Show this help message"

.PHONY: all debug clean test test-task1 test-task2 test-task3 test-task4 \
        test-task1-v test-task2-v test-task3-v test-task4-v \
        test-all check depend help

# Include dependencies if available
-include .depend

