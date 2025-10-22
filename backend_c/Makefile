CC = gcc
CFLAGS = -Iinclude -Wall -O2
SRC = src/main.c src/analyzer.c src/linear_probing.c src/quadratic_probing.c src/chaining.c
BIN = bin/analyzer.exe

all:
	mkdir -p bin results
	$(CC) $(SRC) $(CFLAGS) -o $(BIN)

run:
	./bin/analyzer.exe 1000 results/results.csv

clean:
	rm -rf bin results
