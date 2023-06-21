CC=g++
CFLAGS=-pedantic -Wall -Wextra -Werror -g -ggdb3
OBJ = main.o symbol.o adaptivescanner.o submatrixinfo.o helper.o files.o huffman.o
SRC = main.cpp symbol.cpp adaptivescanner.cpp submatrixinfo.cpp helper.cpp files.cpp huffman.cpp

.PHONY: huff_codec clean all

huff_codec: $(OBJ)
	$(CC) $(CFLAGS) -o huff_codec $(OBJ)

$(OBJ): $(SRC)
	$(CC) -c $(SRC)	

clean:
	rm -f *.o huff_codec

all: clean huff_codec
