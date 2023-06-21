/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 28.4.2023
* Popis: Implementace funkcí pro práci se soubory.
*/

#include "files.h"


/*
* Funkce pro otevření vstupního souboru a načtení jeho obsahu.
*/
vector<uint8_t> read_input(string filename)
{
    vector<uint8_t> input;
    uint8_t pixel;
    ifstream inputFile;

    inputFile.open(filename, ios::in | ios::binary);

    if ( ! inputFile.is_open()) {
        cerr << "Vstupni soubor se nepodarilo otevrit" << endl;

        exit(1);
    }

    while (inputFile.read(reinterpret_cast<char*>(&pixel), sizeof(pixel))) {
        input.push_back(pixel);
    }

    inputFile.close();

    return input;
}


/*
* Funkce pro otevření souboru a zapsání komprimovaných dat. 
* Je využivána k zápisu po statickém průchodu.
*/
void write_output(string filename, uint8_t bitlens_count, vector<uint8_t> bitlens, vector<Symbol>& symbols, vector<bool>& compressed_input)
{
    ofstream output_file;

    output_file.open(filename, ios::out | ios::binary);

    if ( ! output_file.is_open()) {
        cerr << "Vystupni soubor se nepodarilo otevrit" << endl;

        exit(1);
    }

    uint8_t byte = 0;
    uint8_t padding = 0;
    vector<uint8_t> compressed_input_bytes;
    for (int i = 0; i < compressed_input.size(); i++) {
        padding++;
        if (compressed_input[i]) {
            byte |= 1 << (7 - i % 8);
        }
        if (padding == 8) {
            compressed_input_bytes.push_back(byte);
            byte = 0;
            padding = 0;
        }
    }

    if (padding != 0) {
        compressed_input_bytes.push_back(byte);
        padding = 8 - padding;
    }

    /*
    * HEADER
    */
    output_file.write(reinterpret_cast<char*>(&padding), sizeof(padding));
    output_file.write(reinterpret_cast<char*>(&bitlens_count), sizeof(bitlens_count));
    for (int i = 0; i < bitlens.size(); i++) {
        output_file.write(reinterpret_cast<char*>(&bitlens[i]), sizeof(bitlens[i]));
    }
    for (int i = 0; i < symbols.size(); i++) {
        uint8_t symbol_value = symbols[i].get_value();
        output_file.write(reinterpret_cast<char*>(&symbol_value), sizeof(symbol_value));
    }

    /*
    * KOMPRIMOVANE TELO
    */
    for (int i = 0; i < compressed_input_bytes.size(); i++) {
        output_file.write(reinterpret_cast<char*>(&compressed_input_bytes[i]), sizeof(compressed_input_bytes[i]));
    }

    output_file.close();
}


/*
* Funkce pro otevření souboru a zapsání dat ze vstupního vektoru.
* Je využivána při zápisu komprimovaných dat při adaptivním průchodu.
*/
void write_output_adaptive_compressed(string output_filename)
{
    ofstream output_file;

    output_file.open(output_filename, ios::out | ios::binary);

    if ( ! output_file.is_open()) {
        cerr << "Vystupni soubor se nepodarilo otevrit" << endl;

        exit(1);
    }

    for (int i = 0; i < adaptive_compressed_result.size(); i++) {
        output_file.write(reinterpret_cast<char*>(&adaptive_compressed_result[i]), sizeof(adaptive_compressed_result[i]));
    }

    output_file.close();
}


/*
* Funkce pro otevření souboru a zapsání dat ze vstupního vektoru.
* Je využivána při zápisu dat po dekomprimaci.
*
* (Pozn.: funkce je téměř totožná s funkcí write_output_adaptive_compressed(). Zde by dával smysl refactoring a funkce sloučit.
* Pro jednoduchost jsou však ponechány.)
*/
void write_output_decompressed(vector<uint8_t>& output, string output_filename)
{
    ofstream output_file;

    output_file.open(output_filename, ios::out | ios::binary);

    if ( ! output_file.is_open()) {
        cerr << "Vystupni soubor se nepodarilo otevrit" << endl;

        exit(1);
    }

    for (int i = 0; i < output.size(); i++) {
        output_file.write(reinterpret_cast<char*>(&output[i]), sizeof(output[i]));
    }

    output_file.close();
}