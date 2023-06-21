/**
*   Soubor: steg-decode.c
*   IJC-DU1, priklad a)
*   Autor: Vojtěch Mimochodek, xmimoc01, VUT FIT
*   Přeloženo: gcc 6.4.0
*   Datum odevzdání: 20.3.2018
*   Popis: funkce pro dešifrování tajné zprávy
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bit_array.h"
#include "eratosthenes.h"
#include "ppm.h"

int main(int argc, char *argv[])
{

    char POM[500] = {0,};
    unsigned int i = 0;
    unsigned int t = 0;

    if (argc > 2)
    {
        fprintf(stderr,"\nArgumenty mimo první budou ignorovány!\n");
    }
    else if (argc < 2)
    {
        error_exit("Není definovan zdroj obrazku!");
    }

    struct ppm *obrazek;
    if ((obrazek = ppm_read(argv[1])) == NULL)
    {
        error_exit("Chybny format souboru!\n");
    }

    bit_array_create(kod,1000*1000*3);
    bit_array_size(kod) = sizeof(char)*3*obrazek->xsize*obrazek->ysize;

    Eratosthenes(kod);

    for(i = 11; i < bit_array_size(kod); i++)
    {
        if(!bit_array_getbit(kod, i))
        {
            if ((obrazek->data[i] & 1)) POM[t/BITS_IN_CHAR] |= (1<<(t % BITS_IN_CHAR));
            if (t % BITS_IN_CHAR == 7)
            {
                if (POM[t/BITS_IN_CHAR] == '\0')
                {
                    for(i = 0; i < t/BITS_IN_CHAR; i++)
                    if (!isprint(POM[i]))
                    {
                        error_exit("Zpráva obsahuje netisknutelné znaky!");
                        free(obrazek);
                    }

                    printf("%s\n", POM);

                    free(obrazek);
                    return 0;
                }
            }
            t++;
        }
    }
    if (POM[t/BITS_IN_CHAR] != '\0')
    {
        free(obrazek);
        error_exit("Zpráva není spravne ukoncena!");
    }
}
