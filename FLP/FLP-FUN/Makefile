CC=ghc
CFLAGS=--make -o flp22-fun -Wall
OBJ=src/*.hs


.PHONY: all clean fc

all: fun

fun: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ)

clean:
	rm -f src/*.o src/*.hi flp22-fun

fc: clean fun