# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g $(INCLUDES)

#----------------------------------------
# Testing File Specifics
#----------------------------------------
TEST_FILE_DIR = ../../../drivers/core/spi

#----------------------------------------
# Dependency Projects
#----------------------------------------
CONFIG_TARGET = ../../../common/config
MOCK_GPIO_TARGET = ../libs/mocked_libs/gpio

# Object file declarations (wildcards evaluated when used)
CONFIG_OBJS = $(wildcard $(CONFIG_TARGET)/bin/*.o)
MOCK_GPIO_OBJS = $(wildcard $(MOCK_GPIO_TARGET)/bin/*.o)

#----------------------------------------
# Includes
#----------------------------------------
PROJECT_INCLUDES = ../../../include 

#----------------------------------------
# Mock Includes
#----------------------------------------
MOCK_INCLUDES = $(MOCK_GPIO_TARGET)

#----------------------------------------
# Build Targets
#----------------------------------------
TEST_DIR = .
UNITY_DIR = ../libs/unity/src
OUTPUT_DIR = bin
OUTPUT = $(OUTPUT_DIR)/unit_test

# Source files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
UNITY_SRCS = $(wildcard $(UNITY_DIR)/*.c)
TEST_FILE_SRCS = $(wildcard $(TEST_FILE_DIR)/*.c)

# Object files
TEST_FILE_OBJS = $(patsubst $(TEST_FILE_DIR)/%.c, $(OUTPUT_DIR)/%.o, $(TEST_FILE_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(OUTPUT_DIR)/%.o, $(TEST_SRCS))
UNITY_OBJS = $(patsubst $(UNITY_DIR)/%.c, $(OUTPUT_DIR)/%.o, $(UNITY_SRCS))

# Include directories
UNITY_INCLUDES = ../libs/unity/src
INCLUDES = -I$(TEST_FILE_DIR) -I$(PROJECT_INCLUDES) -I$(UNITY_INCLUDES) -I$(MOCK_INCLUDES)

#----------------------------------------
# Build Rules
#----------------------------------------
.PHONY: all clean test build_deps

all: build_deps $(OUTPUT)

build_deps:
	$(MAKE) -C $(CONFIG_TARGET)
	@echo "Building GPIO mock..."
	$(MAKE) -C $(MOCK_GPIO_TARGET)
	@echo "Finished building GPIO mock..."

$(OUTPUT_DIR):
	mkdir -p $@

$(OUTPUT): $(TEST_OBJS) $(TEST_FILE_OBJS) $(UNITY_OBJS) $(CONFIG_OBJS) $(MOCK_GPIO_OBJS) | $(OUTPUT_DIR)
	$(CC) -o $@ $^

$(OUTPUT_DIR)/%.o: $(TEST_FILE_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: $(TEST_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: $(UNITY_DIR)/%.c | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

#----------------------------------------
# Utility Targets
#----------------------------------------
clean:
	rm -rf $(OUTPUT_DIR)
	$(MAKE) -C $(CONFIG_TARGET) clean
	$(MAKE) -C $(MOCK_GPIO_TARGET) clean

test: $(OUTPUT)
	./$(OUTPUT)
