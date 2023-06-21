/**
*   Soubor: error.h
*   IJC-DU1, priklad b)
*   Autor: Vojt�ch Mimochodek, xmimoc01, VUT FIT
*   P�elo�eno: gcc 6.4.0
*   Datum odevzd�n�: 20.3.2018
*   Popis: header file s prototypy funkci pro error.c
*/

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

void warning_msg(const char *fmt, ...);
void error_exit(const char *fmt, ...);

#endif // ERROR_H_INCLUDED
