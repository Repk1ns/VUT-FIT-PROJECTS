/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Hlavičkový soubor pro soubor helper.cpp
*/

#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <vector>

using namespace std;

vector<uint8_t> transpose_matrix(const vector<uint8_t>& matrix, int height, int width);
vector<vector<uint8_t>> transpose_block(const vector<vector<uint8_t>>& block);
vector<uint8_t> mat_to_vector(const vector<vector<uint8_t>>& mat);

#endif
