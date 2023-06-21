# IJC-DU1
# Vojtech Mimochodek, xmimoc01, VUT FIT
# 20.3.2018
# popis: makefile


CC=gcc
CFLAGS= -O2 -std=c99 -pedantic -Wall -Wextra -g


a: primes primes-inline steg-decode


primes: primes.c eratosthenes.o
	$(CC) $(CFLAGS) primes.c eratosthenes.o error.o -lm -o $@
	
primes-inline: primes.c eratosthenes.o
	$(CC) $(CFLAGS) -DUSE_INLINE primes.c eratosthenes.o error.o -lm -o $@
	
steg-decode: steg-decode.c ppm.o eratosthenes.o
	$(CC) $(CFLAGS) steg-decode.c ppm.o eratosthenes.o error.o -lm -o $@
	
eratosthenes.o: eratosthenes.c eratosthenes.h bit_array.h error.o
	$(CC) $(CFLAGS) eratosthenes.c -lm -c -o $@
	
ppm.o: ppm.c ppm.h
	$(CC) $(CFLAGS) ppm.c -lm -c -o $@

error.o: error.c error.h
	$(CC) $(CFLAGS) error.c -lm -c -o $@


clean:
	rm eratosthenes.o ppm.o error.o
	
clean-a: clean;
	rm primes primes-inline steg-decode
