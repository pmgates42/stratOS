# Includes
INCLUDES = ../../../include

# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INCLUDES)

# Config Source Directory
THIS_DIR = .

# Source files
CONFIG_SRCS = $(wildcard $(THIS_DIR)/*.c)
CONFIG_OBJS = $(patsubst $(THIS_DIR)/%.c, bin/%.o, $(CONFIG_SRCS))

# Default target
all: $(THIS_DIR)

bin/%.o: $(THIS_DIR)/%.c | bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -f bin/*.o

.PHONY: all clean
