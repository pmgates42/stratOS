# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g $(INCLUDES)

# Project includes
PROJECT_INCLUDES = ../../../include

# Unit directory
SSCHED_DIR = ../../../ssched
TEST_DIR = .
UNITY_DIR = ../libs/unity/src

# Source files to include
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
UNITY_SRCS = $(wildcard $(UNITY_DIR)/*.c)
# Object files to create
SSCHED_OBJS = $(patsubst $(SSCHED_DIR)/%.c, bin/%.o, $(SSCHED_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, bin/%.o, $(TEST_SRCS))
UNITY_OBJS = $(patsubst $(UNITY_DIR)/%.c, bin/%.o, $(UNITY_SRCS))

# Bin output
OUTPUT_DIR = bin
OUTPUT = $(OUTPUT_DIR)/unit_test_ssched

# Test framework stuff
UNITY_INCLUDES = ../libs/unity/src

# Header files
INCLUDES = -I$(SSCHED_DIR) -I$(PROJECT_INCLUDES) -I$(UNITY_INCLUDES)

# Defines
DEFINES = -DSSCHED_SHOW_DEBUG_DATA -DSSCHED_LOG_TASK_STATS

# Default target
all: $(OUTPUT_DIR) $(OUTPUT)

# Create bin directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Build test
$(OUTPUT): $(SSCHED_OBJS) $(TEST_OBJS) $(UNITY_OBJS)
	$(CC) -o $@ $^

bin/%.o: $(SSCHED_DIR)/%.c | $(OUTPUT_DIR)
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

.PHONY: all clean test
