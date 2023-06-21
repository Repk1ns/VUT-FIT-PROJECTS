/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 25.4.2023
* Popis: Pomocná třída pro získání informací o aktuálně dekomprimované matici.
*/

#include "submatrixinfo.h"

/*
* Pomocná třída pro získání šířky a výšky aktuálně dekomprimované submatice/bloku.
* Na základě šířky a výšky původního obrazu a velikosti standardní velikosti matice
* je dopočítávána výška a šířka pro aktuálně zpracovávanou matici.
*/
SubmatrixInfo::SubmatrixInfo(int original_height, int original_width)
    : original_height_(original_height), original_width_(original_width),
      current_row_(0), current_col_(0) {}


/*
* Metoda pro získání výšky pro matici.
*/
int SubmatrixInfo::get_next_height() {
    int submatrix_height = 32;
    if (current_row_ == get_num_submatrices_height() - 1) {
        submatrix_height = original_height_ % 32;
        if (submatrix_height == 0) {
            submatrix_height = 32;
        }
    }
    return submatrix_height;
}


/*
* Metoda pro získání šířky pro matici.
*/
int SubmatrixInfo::get_next_width() {
    int submatrix_width = 32;
    if (current_col_ == get_num_submatrices_width() - 1) {
        submatrix_width = original_width_ % 32;
        if (submatrix_width == 0) {
            submatrix_width = 32;
        }
    }
    return submatrix_width;
}


/*
* Metoda pro posun k další matici, která následuje za již zpracovanou.
*/
void SubmatrixInfo::move_to_next() {
    current_col_++;
    if (current_col_ >= get_num_submatrices_width()) {
        current_col_ = 0;
        current_row_++;
    }
}


/*
* Pomocná metoda k získání počtu submatic podle výšky.
*/
int SubmatrixInfo::get_num_submatrices_height() {
    return (original_height_ + 31) / 32;
}


/*
* Pomocná metoda k získání počtu submatic podle šířky.
*/
int SubmatrixInfo::get_num_submatrices_width() {
    return (original_width_ + 31) / 32;
}
