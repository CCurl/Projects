# Makefile for SL - Simple Lang

CC = gcc
CFLAGS = -Wall -O2
TARGET = sl

all: $(TARGET)

$(TARGET): sl.c
	$(CC) $(CFLAGS) -o $(TARGET) sl.c

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	./$(TARGET) examples/hello.sl
	./$(TARGET) examples/math.sl

.PHONY: all clean test
