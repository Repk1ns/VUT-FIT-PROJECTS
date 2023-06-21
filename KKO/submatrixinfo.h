/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 25.4.2023
* Popis: Hlavičkový soubor pro soubor submatrixinfo.cpp
*/

#ifndef SUBMATRIX_INFO_H
#define SUBMATRIX_INFO_H

#include <cstdint>

class SubmatrixInfo {
public:
    SubmatrixInfo(int original_height, int original_width);

    int get_next_height();
    int get_next_width();
    void move_to_next();

private:
    int get_num_submatrices_height();
    int get_num_submatrices_width();

    int original_height_;
    int original_width_;
    int current_row_;
    int current_col_;
};

#endif // SUBMATRIX_INFO_H
