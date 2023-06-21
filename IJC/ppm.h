/**
*   Soubor: ppm.h
*   IJC-DU1, priklad a)
*   Autor: Vojtìch Mimochodek, xmimoc01, VUT FIT
*   Pøeloženo: gcc 6.4.0
*   Datum odevzdání: 20.3.2018
*   Popis: header file s prototypy funkcí pro ppm.c
*/


#ifndef PPM_H_INCLUDED
#define PPM_H_INCLUDED

struct ppm
{
    unsigned xsize;
    unsigned ysize;
    char data[];
};

struct ppm * ppm_read(const char * filename);

int ppm_write(struct ppm *p, const char * filename);


#endif // PPM_H_INCLUDED
