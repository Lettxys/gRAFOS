CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c99
LDFLAGS = -lm
TARGET = tsp_solver

all: $(TARGET)

$(TARGET): tsp_solver.c
	$(CC) $(CFLAGS) -o $(TARGET) tsp_solver.c $(LDFLAGS)

clean:
	rm -f $(TARGET) *.tour

test: $(TARGET)
	./$(TARGET) < tulio5.tsp

.PHONY: all clean test
