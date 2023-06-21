/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Hlavičkový soubor pro soubor huffman.cpp
*/

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <functional>
#include <algorithm>
#include <unordered_set>
#include "symbol.h"
#include "adaptivescanner.h"
#include "submatrixinfo.h"
#include "files.h"
#include "helper.h"

using namespace std;

extern vector<uint8_t> adaptive_compressed_result;
extern vector<uint8_t> adaptive_decompressed_result;

vector<uint8_t> set_compress_model(vector<uint8_t> input);
vector<uint8_t> set_decompress_model(vector<uint8_t> input);
vector<int> get_frequencies(vector<uint8_t> input);
vector<Symbol> create_symbols(const vector<int>& frequencies);
void create_canonical_codes(vector<Symbol>& symbols);
uint8_t count_unique_bitlens(vector<Symbol>& symbols);
vector<uint8_t> count_bitlens_for_symbols(vector<Symbol>& symbols, uint8_t bitlens_count);
vector<int> count_bitlens(vector<Symbol> symbols);
void add_to_output(uint8_t direction, uint8_t bitlens_count, vector<uint8_t> bitlens, vector<Symbol>& symbols, vector<bool>& compressed_input);
vector<bool> compress_input(vector<Symbol>& symbols, vector<uint8_t>& input);
void adaptive_compress(vector<uint8_t>& vertical, vector<uint8_t>& horizontal);
void compress(vector<uint8_t>& input, string output_filename);
vector<uint8_t> adaptive_decompress(vector<uint8_t>& input, string output_filename, bool is_model, uint16_t image_width, uint16_t image_height);
vector<uint8_t> decompress(vector<uint8_t>& input, string output_filename);

#endif