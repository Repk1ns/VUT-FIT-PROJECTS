/**
*   Soubor: bit_array.h
*   IJC-DU1, priklad a)
*   Autor: Vojtěch Mimochodek, xmimoc01, VUT FIT
*   Přeloženo: gcc 6.4.0
*   Datum odevzdání: 20.3.2018
*   Popis: makra a inline funkce
*/
#ifndef BIT_ARRAY_H_INCLUDED
#define BIT_ARRAY_H_INCLUDED

#include "error.h"

#define BYTES_IN_LONG (sizeof(long)
#define BITS_IN_CHAR (sizeof(char)<<3)
#define BITS_IN_LONG (sizeof(long)<<3)


typedef unsigned long bit_array_t[];


#define size_bits(typ) (sizeof(typ)<<3)


#define bit_array_create(jmeno_pole,velikost) \
    unsigned long jmeno_pole[1 + (velikost + BITS_IN_LONG) / BITS_IN_LONG] = {velikost, 0}


#define POMOCNA_SETBIT(jmeno_pole, index, vyraz) \
    ((vyraz) ? (jmeno_pole[1 + (index)/size_bits(*jmeno_pole)] |= (1L<<((index)%size_bits(*jmeno_pole)))) \
         : (jmeno_pole[1 + (index)/size_bits(*jmeno_pole)] &= ~(1L<<((index)%size_bits(*jmeno_pole)))))


#ifdef USE_INLINE

    inline unsigned long bit_array_size(bit_array_t jmeno_pole)
    {
        return jmeno_pole[0];
    }

    inline void bit_array_setbit(bit_array_t jmeno_pole,
                            unsigned long index,
                            unsigned int vyraz)
    {
        if ((index) < bit_array_size(jmeno_pole)) /*jsou dosazovany pouze unsigned promenne tudiz neni potreba ((index) >= 0)*/
        {
            POMOCNA_SETBIT(jmeno_pole, index, vyraz);
        }
        else
        {
            error_exit("Index %ld mimo rozsah 0..%ld",
                        (long)index,
                        (long)bit_array_size(jmeno_pole)-1);
        }
    }

    inline int bit_array_getbit(bit_array_t jmeno_pole, unsigned long index)
    {
        if ((index) < bit_array_size(jmeno_pole)) /*jsou dosazovany pouze unsigned promenne tudiz neni potreba ((index) >= 0)*/
        {
            return ((jmeno_pole[1 + (index)/size_bits(*jmeno_pole)]>>((index)%size_bits(*jmeno_pole))) & 1);
        }
        else
        {
            error_exit("Index %ld mimo rozsah 0..%ld",
                        (long)index,
                        (long)bit_array_size(jmeno_pole)-1);
            return -1;
        }
    }

#else


#define bit_array_size(jmeno_pole) jmeno_pole[0]


#define bit_array_setbit(jmeno_pole,index,vyraz) \
    ((index) < bit_array_size(jmeno_pole)) ? /*jsou dosazovany pouze unsigned promenne tudiz neni potreba ((index) >= 0)*/ \
        POMOCNA_SETBIT(jmeno_pole,index,vyraz) : \
            (error_exit("Index %ld mimo rozsah 0..%ld", \
                         (long)index, \
                         (long)bit_array_size(jmeno_pole)-1), 2)


#define bit_array_getbit(jmeno_pole,index)\
    (((index) < bit_array_size(jmeno_pole)) ? \
        ((jmeno_pole[1 + (index)/size_bits(*jmeno_pole)]>>((index)%size_bits(*jmeno_pole))) & 1) : \
            (error_exit("Index %ld mimo rozsah 0..%ld", \
                         (long)index, \
                         (long)bit_array_size(jmeno_pole)-1), 2))


#endif // BIT_ARRAY_H_INCLUDED
#endif
