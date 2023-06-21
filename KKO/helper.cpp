/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Soubor obsahující pomocné funkce, především pro práci s maticemi.
*/

#include "helper.h"

/*
* Pomocná funkce pro transpozici matice
*/
vector<uint8_t> transpose_matrix(const vector<uint8_t>& matrix, int height, int width) 
{
    vector<uint8_t> transposed_matrix(height * width);

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            transposed_matrix[col * height + row] = matrix[row * width + col];
        }
    }

    return transposed_matrix;
}


/*
* Pomocná funkce pro transpozici matice.
* Rozdílem oproti funkci transpose_matrix je, že tato funkce pracuje s maticemi ve formátu 2D vektoru.
*/
vector<vector<uint8_t>> transpose_block(const vector<vector<uint8_t>>& block) 
{
    int height = block.size();
    int width = block[0].size();
    vector<vector<uint8_t>> transposed_block(width, vector<uint8_t>(height));

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            transposed_block[col][row] = block[row][col];
        }
    }

    return transposed_block;
}


/*
* Pomocná funkce pro převod matice uložené ve formátu 2D vektoru na jednoduchý vektor.
*/
vector<uint8_t> mat_to_vector(const vector<vector<uint8_t>>& mat)
{
    vector<uint8_t> result;
    for (const auto& row : mat) {
        result.insert(result.end(), row.begin(), row.end());
    }

    return result;
}
