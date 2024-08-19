# Includes
INCLUDES = ../../include

# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INCLUDES)

# Config Source Directory
CONFIG_DIR = .

# Source files
CONFIG_SRCS = $(wildcard $(CONFIG_DIR)/*.c)
CONFIG_OBJS = $(patsubst $(CONFIG_DIR)/%.c, bin/%.o, $(CONFIG_SRCS))

# Default target
all: $(CONFIG_OBJS)

bin/%.o: $(CONFIG_DIR)/%.c | bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -f bin/*.o

.PHONY: all clean
