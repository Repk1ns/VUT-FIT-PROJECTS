/**
*   Soubor: error.c
*   IJC-DU1, priklad b)
*   Autor: Vojt�ch Mimochodek, xmimoc01, VUT FIT
*   P�elo�eno: gcc 6.4.0
*   Datum odevzd�n�: 20.3.2018
*   Popis: funkce pro ohl�en� chybov�ch stav�
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "error.h"

void warning_msg(const char *fmt, ...)
{

    char vypis[420] = {'\n', 'C', 'H', 'Y', 'B', 'A', ':', ' '};
    unsigned int x = 0;

    va_list argcs;

    do {
    vypis[x+8] = fmt[x];
    x++;
    } while(fmt[x] != '\0');

    va_start(argcs, fmt);
    vfprintf(stderr, vypis, argcs);
    va_end(argcs);
}

void error_exit(const char *fmt, ...)
{
    char vypis[420] = {'\n', 'C', 'H', 'Y', 'B', 'A', ':', ' '};
    unsigned int x = 0;

    va_list argcs;

    do {
    vypis[x+8] = fmt[x];
    x++;
    } while(fmt[x] != '\0');

    va_start(argcs, fmt);
    vfprintf(stderr, vypis, argcs);
    va_end(argcs);

    exit(1);
}
