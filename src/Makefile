#!/usr/bin/env make

CC = gcc
RM = rm -f

CFLAGS += -Wall -Wextra -Wpedantic -O2 -fstack-protector-strong
LDFLAGS += -lrt

SRCS = kalyna.c main.c tables.c
OBJS = kalyna.o main.o tables.o
TARGET = kalyna
BENCH_TARGET = kalyna-benchmark


default: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

test: $(TARGET)
	./$(TARGET)

benchmark: $(BENCH_TARGET)
	./$(BENCH_TARGET)

$(BENCH_TARGET): kalyna.o tables.o benchmark.o
	$(CC) -o $@ kalyna.o tables.o benchmark.o $(LDFLAGS)

clean:
	$(RM) *.o $(TARGET) $(BENCH_TARGET)

tables.c: tables.h kalyna.h

main.c:   kalyna.h transformations.h

kalyna.c:  transformations.h tables.h

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

#
