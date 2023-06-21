/**
*   Soubor: primes.c
*   IJC-DU1, priklad a)
*   Autor: Vojt�ch Mimochodek, xmimoc01, VUT FIT
*   P�elo�eno: gcc 6.4.0
*   Datum odevzd�n�: 20.3.2018
*   Popis: funkce pro vyps�n� posledn�ch 10 prvo��sel z 222 milion�
*/


#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "bit_array.h"
#include "eratosthenes.h"

#define PRVOCISEL 10
#define LIMIT 222000000 + 1


int main()
{
  unsigned long int i, j = 0;

  bit_array_create(p,LIMIT);

  Eratosthenes(p);

  for(i = bit_array_size(p)-1; (i > 2) && (j < PRVOCISEL) ; i--)
    {
        if(!bit_array_getbit(p, i))
        j++;
    }

  while(i < bit_array_size(p))
  {
    if(!bit_array_getbit(p, i))
        printf("%lu\n", i);

    i++;
  }

  return 0;
}
