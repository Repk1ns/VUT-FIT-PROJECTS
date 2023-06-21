/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Třída pro reprezentaci symbolu.
*/

#include "symbol.h"

/*
* Třída pro reprezentaci symbolu vyskytujícího se ve vstupním souboru.
* Symbol nese informace o jeho nezakomprimované hodnotě (0-255),
* o frekvencích výskytu symbolu v souboru, délce kódového slova (bitlens) 
* a kanonickém kódu pro tento symbol.
*/
Symbol::Symbol(uint8_t value, int frequencies, int bitlen)
{
    this->value = value;
    this->frequencies = frequencies;
    this->bitlen = bitlen;
}

uint8_t Symbol::get_value()
{
    return this->value;
}

int Symbol::get_frequencies()
{
    return this->frequencies;
}

int Symbol::get_bitlen()
{
    return this->bitlen;
}

void Symbol::set_bitlen(int bitlen)
{
    this->bitlen = bitlen;
}

vector<bool> Symbol::get_code()
{
    return this->code;
}

void Symbol::add_to_code(bool bit)
{
    this->code.insert(this->code.begin(), bit);
}