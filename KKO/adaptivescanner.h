/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 25.4.2023
* Popis: Hlavičkový soubor pro třídu AdaptiveScanner
*/

#ifndef ADAPTIVE_SCANNER_H
#define ADAPTIVE_SCANNER_H

#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

class AdaptiveScanner {
public:
    AdaptiveScanner(vector<uint8_t> input, int width, int height, int blockSize = 16);

    vector<vector<uint8_t>> get_next_mat();
    vector<uint8_t> reconstruct_image_from_blocks(const vector<uint8_t>& input);


private:
    vector<uint8_t> input_;
    int width_;
    int height_;
    int blockSize_;
    int currentX_;
    int currentY_;
};

#endif
