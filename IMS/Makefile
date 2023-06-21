CC=g++
CFLAGS=-pedantic -Wall -Wextra -Werror -g
OBJ = main.o
SRC = main.cpp

.PHONY: all clean run

all: project

project: $(OBJ)
	$(CC) $(CFLAGS) -o main $(OBJ) -lsimlib -lm

$(OBJ): $(SRC)
	$(CC) -c $(SRC)

clean:
	rm -f *.o main

run:
	./main -c 3 -o 100 -s 12 -f 5