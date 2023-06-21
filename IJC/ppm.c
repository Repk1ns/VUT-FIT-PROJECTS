/**
*   Soubor: ppm.c
*   IJC-DU1, priklad b)
*   Autor: Vojtěch Mimochodek, xmimoc01, VUT FIT
*   Přeloženo: gcc 6.4.0
*   Datum odevzdání: 20.3.2018
*   Popis: funkce pro načtení a ukládání obrázku
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppm.h"
#include "error.h"

struct ppm * ppm_read(const char * filename)
{
    FILE *soubor_cteni;
    if ((soubor_cteni = fopen(filename,"rb")) == NULL)
    {
        warning_msg("Obrazek se nepodarilo otevrit!");
        return NULL;
    }

    unsigned int i = 0;
    char pom[40] = {0,};

    if (fscanf(soubor_cteni,"%c%c%c", &pom[0], &pom[1], &pom[2]) != 3 || strcmp(pom,"P6\n"))
    {
        warning_msg("Obrázek neodpovida formatu!");
        return NULL;
    }

    do {
        if (fscanf(soubor_cteni,"%c", &pom[i]) == EOF)
        {
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }
        if (pom[i] == '\0' || i > 38)
        {
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }
        }
        while(pom[i++] != '\n');
        pom[i] = '\0';



    struct ppm *obrazek;
    {
        unsigned int x, y;
        if (sscanf(pom,"%u %u", &x, &y) < 2 )
        {
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }
        if (x*y > 1000*1000)
        {
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }


    obrazek = (struct ppm *) malloc(sizeof(struct ppm)+sizeof(char)*x*y*3); //alokace mista pro obrazek
    if (obrazek == NULL)
    {
        warning_msg("Chybný formát obrázku!");
        return NULL;
    }
    obrazek->xsize = x;
    obrazek->ysize = y;
    }

    i = 0;
    do {
        if (fscanf(soubor_cteni,"%c", &pom[i]) != 1)
        {
            free(obrazek);
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }
        if (pom[i] == '\0' || i > 38)
        {
            free(obrazek);
            warning_msg("Chybný formát obrázku!");
            return NULL;
        }
    }
    while(pom[i++] != '\n');
    pom[i] = '\0';


    for(i = 0; i < (obrazek->xsize*obrazek->ysize*3); i++)
    {
        if (fscanf(soubor_cteni,"%c", &(obrazek->data[i])) != 1)
        {
        free(obrazek);
        warning_msg("Chybný formát obrázku!");
        return NULL;
        }
    }

    if (fclose(soubor_cteni))
    {
        warning_msg("Soubor nelze uzavřít!");
    }

    return obrazek;
}

//zde zacina funkce ppm_write
int ppm_write(struct ppm *p, const char * filename)
{

    FILE *soubor_zapis = fopen(filename, "wb");
    if (soubor_zapis == NULL)
    {
        warning_msg("Soubor %s se nepodařilo otevřít!\n", filename);
        return -1;
    }

    if (fprintf(soubor_zapis,"%s\n%u %u\n%s\n","P6",p->xsize, p->ysize, "255") < 0)
    {
        warning_msg("Soubor %s se nepodařilo otevřít!\n", filename);
        return -1;
    }

    for(unsigned i = 0; i < (p->xsize*p->ysize*3); i++)
    {
    if (fprintf(soubor_zapis,"%c",p->data[i]) < 0)
    {
        warning_msg("Soubor %s se nepodařilo otevřít!\n", filename);
        return -1;
    }
    }

    if (fclose(soubor_zapis))
    {
        warning_msg("Soubor %s se nepodařilo otevřít!\n", filename);
        return -1;
    }

    return 0;
}
