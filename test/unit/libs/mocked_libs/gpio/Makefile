# Includes
INCLUDES = ../../../../../include

# Compiler definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(INCLUDES)

# Config Source Directory
THIS_DIR = .

# Source files
THIS_SRCS = $(wildcard $(THIS_DIR)/*.c)
THIS_OBJS = $(patsubst $(THIS_DIR)/%.c, bin/%.o, $(THIS_SRCS))

# Default target
all: $(THIS_OBJS)

bin/%.o: $(THIS_DIR)/%.c | bin
	$(CC) $(CFLAGS) -c -o $@ $<

bin:
	mkdir -p bin

clean:
	rm -f bin/*.o

.PHONY: all clean
