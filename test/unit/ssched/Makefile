# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g $(INCLUDES)

# Project includes
PROJECT_INCLUDES = ../../../include

# Unit directory
SSCHED_DIR = ../../../ssched
TEST_DIR = .

# Source files to include
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

# Object files to create
SSCHED_OBJS = $(patsubst $(SSCHED_DIR)/%.c, bin/%.o, $(SSCHED_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, bin/%.o, $(TEST_SRCS))

# Bin output
OUTPUT_DIR = bin
OUTPUT = $(OUTPUT_DIR)/unit_test_ssched

# Header files
INCLUDES = -I$(SSCHED_DIR) -I$(PROJECT_INCLUDES) # include all project header files

# Default target
all: $(OUTPUT_DIR) $(OUTPUT)

# Create bin directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Build test
$(OUTPUT): $(SSCHED_OBJS) $(TEST_OBJS)
	$(CC) -o $@ $^

bin/%.o: $(SSCHED_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

bin/%.o: $(TEST_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OUTPUT_DIR)/*.o $(OUTPUT)
	rm -rf $(OUTPUT_DIR)

# Run the tests
test: $(OUTPUT)
	./$(OUTPUT)

.PHONY: all clean test
