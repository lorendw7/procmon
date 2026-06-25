# Makefile for procmon
# Usage:
#   make          build ./procmon
#   make run      build and run with default options
#   make clean    remove the built binary

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=c11
TARGET  := procmon
SRC     := src/procmon.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
