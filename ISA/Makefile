CC=g++
CFLAGS=-pedantic -Wall -Wextra -Werror -g
OBJ = errors.o parse.o input.o requests.o connection.o main.o
SRC = errors.cpp parse.cpp input.cpp requests.cpp connection.cpp main.cpp

.PHONY: all clean test

all: isabot

isabot: $(OBJ)
	$(CC) $(CFLAGS) -o isabot $(OBJ) -lssl -lcrypto

$(OBJ): $(SRC)
	$(CC) -c $(SRC)	

clean:
	rm -f *.o isabot

test:
	php tester.php