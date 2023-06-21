/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 10.4.2023
* Popis: Hlavní soubor programu. Zajišťuje rozparsování vstupních argumentů
* a volání funkcí pro kompresi a dekompresi.
*/

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <vector>
#include <string>
#include "symbol.h"
#include "adaptivescanner.h"
#include "submatrixinfo.h"
#include "files.h"
#include "helper.h"
#include "huffman.h"

using namespace std;


void printHelp()
{
    cout << "---------------------" << endl;
    cout << "CZ: Kompresor obrazových dat - Huffmanovo kódování" << endl;
    cout << "NAPOVEDA:" << endl;
    cout << endl;
    cout << "Aplikaci je možné spustit s následujícími parametry:" << endl;
    cout << "[-c] : pro kompresi vstupního souboru" << endl;
    cout << "[-d] : pro dekompresi vstupního souboru" << endl;
    cout << "[-m] : pro aktivaci modelu" << endl;
    cout << "[-a] : pro aktivaci adaptivního skenování" << endl;
    cout << "[-i] <nazev_souboru> : pro zadání vstupního souboru" << endl;
    cout << "[-o] <nazev_souboru> : pro zadani výstupního souboru" << endl;
    cout << "[-w] <šířka_obrazu> : pro specifikaci šířky obrazu, kde šířka >= 1 (pouze s -d)" << endl;
    cout << "[-h] : pro vypsání nápovědy" << endl;
    cout << endl;
    cout << "---------------------" << endl;
}


/*
* Hlavní funkce programu.
*
* Program nejprve pomocí knihovny getopt rozparsuje argumenty příkazové řádky.
* Následně se dle zadaných parametrů provede komprese nebo dekomprese.
*/
int main(int argc, char *argv[])
{
    int opt;
    bool is_model = false;
    bool is_adaptive = false;
    bool is_compression = false;
    bool is_decompression = false;
    int width = 0;
    string input_filename;
    string output_filename;

    /*
    * Parsování argumentů pomocí getopt
    */
    while ((opt = getopt(argc, argv, "cdmai:o:w:h")) != -1) {
        switch (opt) {
            case 'c':
                is_compression = true;
                break;
            case 'd':
                is_decompression = true;
                break;
            case 'm':
                is_model = true;
                break;
            case 'a':
                is_adaptive = true;
                break;
            case 'i':
                input_filename = optarg;
                break;
            case 'o':
                output_filename = optarg;
                break;
            case 'w':
                width = atoi(optarg);
                if (width < 1) {
                    cout << "Chyba: šířka obrazu musí být >= 1" << endl;

                    exit(1);
                }

                break;
            case 'h':
                printHelp();

                return 0;
                break;
            default:
                std::cerr << "Chybně zadaný argument: -" << (char)optopt << std::endl;

                exit(1);
        }
    }

    if (is_compression && is_decompression) {
        cout << "Chyba: nelze kombinovat parametry -c a -d" << endl;

        exit(1);
    }

    if ( ! is_compression && ! is_decompression) {
        cout << "Chyba: musí být zadán parametr -c nebo -d" << endl;

        exit(1);
    }

    if (input_filename.empty()) {
        cout << "Chyba: musí být zadán parametr -i <nazev_souboru>" << endl;

        exit(1);
    }

    if (output_filename.empty()) {
        cout << "Chyba: musí být zadán parametr -o <nazev_souboru>" << endl;

        exit(1);
    }

    vector<uint8_t> input = read_input(input_filename);

    /*
    * KOMPRESE
    *
    * V případě statického průchodu je načten vstupní soubor, který je celý zakomprimován. Případně je před kompresí aplikován model
    *
    * V případě adaptivního průchodu je provedena kontrola zadané šířky obrazu a dopočítána výška. Šířka a výška je uložena do hlavičky
    * komprimovaného souboru. Následně je vstupní soubor rozdělen na bloky o velikosti 32x32 bytů. 
    * Postupně je každý blok zakomprimován jak v horizontálním směru tak vertikálním. Pro vertikální směr
    * je blok/submatice nejprve transponována. Podle toho, jaký průchod pro danou submatici dává lepší výsledek, takový průchod je použit.
    * Je-li zadán parametr -m, před komprimací každého bloku je nejprve aplikován model.
    * Výsledek všech zakomprimovaných bloků je uložen do výstupního souboru.
    */
    if (is_compression) {

        if (is_adaptive) {
            int pic_width = width;

            if ( ! pic_width) {
                cerr << "Chyba: chybí zadaná šířka obrazu pro adaptivní kódování" << endl;
                exit(1);
            }

            int input_size = input.size();
            if (input_size % pic_width != 0) {
                cerr << "Chyba: obraz nemá vhodné rozměry. Matice není souměrná a výška není korektní." << endl;
                exit(1);
            }

            int pic_height = input.size() / pic_width;
            
            uint16_t width_to_header = pic_width;
            uint16_t height_to_header = pic_height;
            adaptive_compressed_result.push_back(static_cast<uint8_t>(width_to_header & 0xFF));
            adaptive_compressed_result.push_back(static_cast<uint8_t>((width_to_header >> 8) & 0xFF)); 
            adaptive_compressed_result.push_back(static_cast<uint8_t>(height_to_header & 0xFF));
            adaptive_compressed_result.push_back(static_cast<uint8_t>((height_to_header >> 8) & 0xFF)); 

            AdaptiveScanner scanner(input, pic_width, pic_height, 32);

            vector<vector<uint8_t>> block;
            while ( ! (block = scanner.get_next_mat()).empty()) {
                vector<vector<uint8_t>> vertical_block = transpose_block(block);
                vector<uint8_t> simplified_block = mat_to_vector(block);
                vector<uint8_t> simplified_vertical_block = mat_to_vector(vertical_block);

                if (is_model) {
                    simplified_block = set_compress_model(simplified_block);
                    simplified_vertical_block = set_compress_model(simplified_vertical_block);
                }
                adaptive_compress(simplified_block, simplified_vertical_block);
            }
            write_output_adaptive_compressed(output_filename);
        } else {
            if (is_model) {
                input = set_compress_model(input);
            }
            compress(input, output_filename);
        }
    }

    /*
    * DEKOMPRESE
    * 
    * V případě statického průchodu se provede dekomprese dle algoritmu a dekomprimovaný obsah je uložen do výstupního souboru
    * 
    * V případě adaptivního průchodu je vyčtena šířka a výška původního obrazu a je provedena dekomprese po částech.
    * Dekomprimované submatice jsou rekonstruovány do původního obrazu. Výsledek je zapsán do výstupního souboru.
    */
    vector<uint8_t> decompressed;
    if (is_decompression) {

        if (is_adaptive) {
            uint8_t lower_byte_width = input[0];
            uint8_t upper_byte_width = input[1];
            uint16_t original_width = (static_cast<uint16_t>(upper_byte_width) << 8) | lower_byte_width;
            uint8_t lower_byte_height = input[2];
            uint8_t upper_byte_height = input[3];
            uint16_t original_height = (static_cast<uint16_t>(upper_byte_height) << 8) | lower_byte_height;
            input.erase(input.begin(), input.begin() + 4);

            decompressed = adaptive_decompress(input, output_filename, is_model, original_width, original_height);

            vector<uint8_t> empty_input;
            AdaptiveScanner scanner(empty_input, original_width, original_height, 32);

            vector<uint8_t> decompressed_image = scanner.reconstruct_image_from_blocks(decompressed);

            write_output_decompressed(decompressed_image, output_filename);
        } else {
            decompressed = decompress(input, output_filename);
            if (is_model) {
                decompressed = set_decompress_model(decompressed);
                write_output_decompressed(decompressed, output_filename);
            } else {
                write_output_decompressed(decompressed, output_filename);
            }
        }
    }

    return 0;
}