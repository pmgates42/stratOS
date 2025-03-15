# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g $(INCLUDES)

# Project includes
PROJECT_INCLUDES = ../../../include

#----------------------------------------
# Testing File Specifics
#----------------------------------------

# Unit directory
TEST_FILE_DIR = ../../../drivers/core/spi

#----------------------------------------
# Projects to include
#----------------------------------------
CONFIG_TARGET = ../../../common/config
CONFIG_MAKE_TARGET:
	$(MAKE) -C $(CONFIG_TARGET)

CONFIG_OBJS = $(wildcard ../../../common/config/bin/*.o)

MOCK_GPIO_TARGET = ../libs/mocked_libs/gpio
MOCK_GPIO_MAKE_TARGET:
	@echo "Running make in MOCK_GPIO_TARGET..."
	$(MAKE) -C $(MOCK_GPIO_TARGET)

MOCK_GPIO_OBJS = $(wildcard ../libs/mocked_libs/gpio/bin/*.o)

#----------------------------------------
# Boilerplate
#----------------------------------------

TEST_DIR = .
UNITY_DIR = ../libs/unity/src

# Source files to include
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
UNITY_SRCS = $(wildcard $(UNITY_DIR)/*.c)

TEST_FILE_SRCS = $(wildcard $(TEST_FILE_DIR)/*.c)

# Object files to create
TEST_FILE_OBJS = $(patsubst $(TEST_FILE_DIR)/%.c, bin/%.o, $(TEST_FILE_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, bin/%.o, $(TEST_SRCS))
UNITY_OBJS = $(patsubst $(UNITY_DIR)/%.c, bin/%.o, $(UNITY_SRCS))

# Bin output
OUTPUT_DIR = bin
OUTPUT = $(OUTPUT_DIR)/unit_test

# Test framework stuff
UNITY_INCLUDES = ../libs/unity/src

# Header files
INCLUDES = -I$(TEST_FILE_DIR) -I$(PROJECT_INCLUDES) -I$(UNITY_INCLUDES)

# Default target
all: CONFIG_MAKE_TARGET MOCK_GPIO_MAKE_TARGET $(OUTPUT_DIR) $(OUTPUT)

# Create bin directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Build test
$(OUTPUT): $(TEST_OBJS) $(TEST_FILE_OBJS) $(UNITY_OBJS) $(CONFIG_OBJS) $(MOCK_GPIO_OBJS)
	$(CC) -o $@ $^

bin/%.o: $(TEST_FILE_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

bin/%.o: $(TEST_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(DEFINES) $(CFLAGS) -c -o $@ $<

bin/%.o: $(UNITY_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OUTPUT_DIR)/*.o $(OUTPUT)
	rm -rf $(OUTPUT_DIR)

# Run the tests
test: $(OUTPUT)
	./$(OUTPUT)

.PHONY: all clean test CONFIG_MAKE_TARGET MOCK_GPIO_MAKE_TARGET
