/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 25.4.2023
* Popis: Třída AdaptiveScanner slouží ke správnému zpracování obrazu po blocích.
*/

#include "adaptivescanner.h"

/*
* Konstruktor třídy AdaptiveScanner
*
* Třída AdaptiveScanner slouží ke správnému zpracování obrazu po blocích.
*/
AdaptiveScanner::AdaptiveScanner(vector<uint8_t> input, int width, int height, int blockSize)
    : input_(input), width_(width), height_(height), blockSize_(blockSize),
      currentX_(0), currentY_(0) {}


/*
* Tato metoda slouží k získání bloku ze zpracovávaného obrazu.
* 
* Metoda je iterativně volána, přičemž vrací vždy daný blok. Jakmile vrácena prázdná matice
* je celý obraz zpracován.
*/
vector<vector<uint8_t>> AdaptiveScanner::get_next_mat() {
    if (currentY_ >= height_) {
        return vector<vector<uint8_t>>(); // Vrátí prázdný 2D vektor, pokud už byl celý obraz zpracován
    }

    int blockWidth = min(blockSize_, width_ - currentX_);
    int blockHeight = min(blockSize_, height_ - currentY_);

    vector<vector<uint8_t>> block(blockHeight, vector<uint8_t>(blockWidth));

    for (int y = 0; y < blockHeight; ++y) {
        for (int x = 0; x < blockWidth; ++x) {
            int index = (currentY_ + y) * width_ + (currentX_ + x);
            block[y][x] = input_[index];
        }
    }

    currentX_ += blockSize_;
    if (currentX_ >= width_) {
        currentX_ = 0;
        currentY_ += blockSize_;
    }

    return block;
}


/*
* Tato metoda slouží k rekonstrukci bloků z dekomprimovaného souboru.
*
* Vstupem je obsah vektoru, ve kterém je obsah celého dekomprimovaného souboru. Tento soubor však obsahuje dekomprimované bloky
* přímo za sebou. Pro správné zpracování je potřeba bloky uspořádat na správná místa v obrazu.
*/
vector<uint8_t> AdaptiveScanner::reconstruct_image_from_blocks(const vector<uint8_t>& input) {
    currentX_ = 0;
    currentY_ = 0;

    vector<uint8_t> result(width_ * height_);

    for (size_t i = 0; i < input.size();) {
        int blockWidth = min(blockSize_, width_ - currentX_);
        int blockHeight = min(blockSize_, height_ - currentY_);
        vector<vector<uint8_t>> block(blockHeight, vector<uint8_t>(blockWidth));

        for (int y = 0; y < blockHeight; ++y) {
            for (int x = 0; x < blockWidth; ++x) {
                block[y][x] = input[i++];
            }
        }

        for (int y = 0; y < blockHeight; ++y) {
            for (int x = 0; x < blockWidth; ++x) {
                int index = (currentY_ + y) * width_ + (currentX_ + x);
                result[index] = block[y][x];
            }
        }

        currentX_ += blockSize_;
        if (currentX_ >= width_) {
            currentX_ = 0;
            currentY_ += blockSize_;
        }
    }

    return result;
}
