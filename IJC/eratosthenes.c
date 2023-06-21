/**
*   Soubor: eratosthenes.c
*   IJC-DU1, priklad a)
*   Autor: Vojtech Mimochodek, xmimoc01, VUT FIT
*   Pøeloženo: gcc 6.4.0
*   Datum odevzdání: 20.3.2018
*   Popis: funkce pro algoritmus - Eratestonovo síto
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bit_array.h"
#include "eratosthenes.h"


void Eratosthenes(bit_array_t pole)
{


    unsigned long velikost = bit_array_size(pole);
    unsigned long limit = (unsigned long) sqrt(bit_array_size(pole));

    for(unsigned long i = 2; i < limit; i++ )
    {
        if(bit_array_getbit(pole, i) == 0)
        {
            for(unsigned long j = 2; (j * i) < velikost; j++ )
            {
                bit_array_setbit(pole, j*i, 1);
            }
        }
    }

}

