/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Hlavičkový soubor pro k souboru symbol.cpp
*/

#ifndef symbol
#define symbol

#include <iostream>
#include <vector>

using namespace std;

class Symbol
{
    private:
        uint8_t value;
        int frequencies;
        int bitlen;
        vector<bool> code;
    
    public:
        Symbol(uint8_t value, int frequencies, int bitlen);
        uint8_t get_value();
        int get_frequencies();
        int get_bitlen();
        void set_bitlen(int bitlen);
        vector<bool> get_code();
        void add_to_code(bool bit);
};

#endif