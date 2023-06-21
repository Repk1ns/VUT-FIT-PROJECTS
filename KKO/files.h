/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 28.4.2023
* Popis: Hlavičkový soubor pro soubor files.cpp
*/


#ifndef FILES_H
#define FILES_H

#include <iostream>
#include <fstream>
#include <vector>
#include "symbol.h"
#include "huffman.h"

using namespace std;

vector<uint8_t> read_input(string filename);

void write_output(string filename, uint8_t bitlens_count, vector<uint8_t> bitlens, vector<Symbol>& symbols, vector<bool>& compressed_input);

void write_output_adaptive_compressed(string output_filename);

void write_output_decompressed(vector<uint8_t>& output, string output_filename);

#endif