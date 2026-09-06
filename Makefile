CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c99 -Iinclude
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS) -O3 -DNDEBUG

SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
EXAMPLE_DIR = examples
BIN_DIR = bin

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET_EXT = .exe
    MKDIR = cmd /C "if not exist $(BIN_DIR) mkdir $(BIN_DIR)"
    RMDIR = cmd /C "if exist $(BIN_DIR) del /Q /F $(BIN_DIR)\* 2>NUL"
else
    TARGET_EXT =
    MKDIR = mkdir -p $(BIN_DIR)
    RMDIR = rm -rf $(BIN_DIR)
endif

LIB_SRC = $(SRC_DIR)/hash_table.c
TEST_SRC = $(TEST_DIR)/test_hash_table.c
DEMO_SRC = $(EXAMPLE_DIR)/demo.c

TEST_BIN = $(BIN_DIR)/test_runner$(TARGET_EXT)
DEMO_BIN = $(BIN_DIR)/demo$(TARGET_EXT)

.PHONY: all check test clean demo

all: $(TEST_BIN) $(DEMO_BIN)

$(BIN_DIR):
	@$(MKDIR)

$(TEST_BIN): $(LIB_SRC) $(TEST_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^

$(DEMO_BIN): $(LIB_SRC) $(DEMO_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_RELEASE) -o $@ $^

check: $(TEST_BIN)
	$(TEST_BIN)

test: check

demo: $(DEMO_BIN)
	$(DEMO_BIN)

clean:
	@$(RMDIR)
