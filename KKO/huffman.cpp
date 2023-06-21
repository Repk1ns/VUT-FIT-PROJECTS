/**
* Komprese a kódování dat 2022/2023
* Autor: Vojtěch Mimochodek (xmimoc01)
* Datum: 15.4.2023
* Popis: Soubor obsahující naimplementované funkce pro kompresi a dekompresi
* pomocí Huffmanova kódování. Naimplementován je například algoritmus Hirschberg-Sieminski.
*/

#include "huffman.h"

/*
* Pomocné globální proměnné pro ukládání výsledků z adaptivní komprese a dekomprese
*/
vector<uint8_t> adaptive_compressed_result;
vector<uint8_t> adaptive_decompressed_result;


/*
* Tato funkce aplikuje model - diferenci sousedů - před kompresí
*/
vector<uint8_t> set_compress_model(vector<uint8_t> input)
{
    uint8_t tmp, htmp;

    tmp = input[0];
    for(int i = 1; i < input.size(); i++) {

        htmp = input[i];
        input[i] = input[i] - tmp;
        tmp = htmp;
    }

    return input;
}


/*
* Tato funkce aplikuje model - diferenci sousedů - po dekompresi
*/
vector<uint8_t> set_decompress_model(vector<uint8_t> input)
{
    for(int i = 1; i < input.size(); i++) {
        input[i] = input[i - 1] + input[i];
    }

    return input;
}


/*
* Tato funkce spočítá frekvence jednotlivých symbolů, které se v daném vstupu vyskytují.
* Vektor je inicializován na 256 hodnot, kde každý index symbolizuje jeden symbol v rozmezí 0-255.
* Hodnota na daném indexu pak udává, kolikrát se daný symbol vyskytl.
*/
vector<int> get_frequencies(vector<uint8_t> input)
{
    vector<int> frequencies(256);

    for (int i = 0; i < input.size(); i++) {
        frequencies[input[i]]++;
    }

    return frequencies;
}


/*
* Pomocná funkce, která na základně napočítaných frekvencí jednotlivých symbolů vytvoří vektor naplněný objekty typu Symbol.
* Vektor symbolů obsahuje pouze ty symboly, které se v daném vstupu vyskytují.
*/
vector<Symbol> create_symbols(const vector<int>& frequencies)
{
    vector<Symbol> symbols;

    for (int i = 0; i < frequencies.size(); i++) {
        if (frequencies[i] > 0) {
            symbols.push_back(Symbol(i, frequencies[i], 0));
        }
    }

    return symbols;
}


/*
* Tato funkce na základě bitových délek slov vytvoří kanonické kódy Huffmanova kódování.
* Každý symbol z vektoru symbolů má svůj kanonický kód uložený.
*/
void create_canonical_codes(vector<Symbol>& symbols)
{
    uint64_t ci = 0;
    int delta = 0;

    while (symbols[0].get_code().size() < symbols[0].get_bitlen()) {
        symbols[0].add_to_code(false);
    }

    for (int i = 1; i < symbols.size(); i++) {

        delta = symbols[i].get_bitlen() - symbols[i - 1].get_bitlen();
        uint64_t canonical_code = (ci + 1) << delta;
        ci = canonical_code;

        while (canonical_code != 0) {
            bool bit = canonical_code % 2;
            symbols[i].add_to_code(bit);
            canonical_code /= 2;
        }

        while (symbols[i].get_code().size() < symbols[i].get_bitlen()) {
            symbols[i].add_to_code(false);
        }

    }
}


/*
* Pomocná funkce, která spočítá počet unikátních bitových délek slov.
*/
uint8_t count_unique_bitlens(vector<Symbol>& symbols)
{
    int last = symbols.size() - 1;
    uint8_t bitlens_size = symbols[last].get_bitlen();

    return bitlens_size;
}


/*
* Tato funkce spočítá kolik symbolů  se kterými se pracuje má danou bitovou délku.
* Tyto bitové délky jsou pak uloženy v hlavičce komprimovaného souboru. (Popř. v hlavičkách každého bloku).
*/
vector<uint8_t> count_bitlens_for_symbols(vector<Symbol>& symbols, uint8_t bitlens_count)
{
    vector<uint8_t> bitlens_for_symbols(bitlens_count);

    int prev_bitlen = symbols[0].get_bitlen();
    int counter = 1;
    for (int i = 1; i < symbols.size(); i++) {
        if (symbols[i].get_bitlen() == prev_bitlen) {
            counter++;
        } else {
            bitlens_for_symbols[prev_bitlen - 1] = counter;
            counter = 1;
            prev_bitlen = symbols[i].get_bitlen();
        }
    }
    
    // !!!
    // V případě, že se vyskytují všechny symboly o stejné délce a délka je tedy 256, 
    // ukládá se do hlavičky jako 0. (0 nenastane a je to efektivnější než použití uint16_t)
    // !!!
    bitlens_for_symbols[prev_bitlen - 1] = counter == 256 ? 0 : counter;

    return bitlens_for_symbols;
}


/*
* Tato funkce realizuje algoritmus Hirschberg-Sieminski pro výpočet bitových délek slov.
*/
vector<int> count_bitlens(vector<Symbol> symbols)
{
    const int left_size = symbols.size();
    const int right_size = left_size * 2;

    vector<int> min_heap_right(right_size);
    vector<int> min_heap_left(left_size);

    int m = left_size;

    for (int i = 0 + left_size; i < right_size; i++) {
        min_heap_right[i] = symbols[i - left_size].get_frequencies();
    }

    for (int i = 0; i < left_size; i++) {
        min_heap_left[i] = i + left_size;
    }

    auto cmp = [&min_heap_right](int a, int b) {
        return min_heap_right[a] > min_heap_right[b];
    };

    make_heap(min_heap_left.begin(), min_heap_left.end(), cmp);

    int first_pop = 0;
    int second_pop = 0;
    int value1 = 0;
    int value2 = 0;
    int position = left_size;
    while (m  > 1) {
        position -= 1;
        first_pop = min_heap_left.front();
        pop_heap(min_heap_left.begin(), min_heap_left.end(), cmp); min_heap_left.pop_back();
        second_pop = min_heap_left.front();
        pop_heap(min_heap_left.begin(), min_heap_left.end(), cmp); min_heap_left.pop_back();

        value1 = min_heap_right[first_pop];
        value2 = min_heap_right[second_pop];

        min_heap_right[position] = value1 + value2;
        min_heap_right[first_pop] = position;
        min_heap_right[second_pop] = position;

        min_heap_left.push_back(position);
        push_heap(min_heap_left.begin(), min_heap_left.end(), cmp);
        m--;
    }

    vector<int> bitlen(left_size);
    for (int i = 0; i < left_size; i++) {

        int j;
        if (min_heap_right.size() == 2) {
            j = 1;
        } else {
            j = min_heap_right[left_size + i];
        }

        int l = 1;

        while (j > 1) {
            j = min_heap_right[j];
            l += 1;
        }
        bitlen[i] = l;
    }

    return bitlen;
}


/*
* Tato funkce připraví hlavičku pro každý blok u adaptivního přístupu.
*
* V hlavičce jsou uloženy informace o konkrétním zakomprimovaném bloku.
*
* Formát hlavičky: 
* [padding - 8bit]
* [direction - 8bit]
* [data_counts - 16bit]
* [bitlens_count - 8bit]
* [bitlens - bitlens_count * 8bit]
* [alphabet - sum(bitlens) * 8bit]
* [comprimed_data - Nbit]
*/
void add_to_output(uint8_t direction, uint8_t bitlens_count, vector<uint8_t> bitlens, vector<Symbol>& symbols, vector<bool>& compressed_input)
{
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

    uint16_t data_size = compressed_input_bytes.size();
    adaptive_compressed_result.push_back(padding);
    adaptive_compressed_result.push_back(direction);
    adaptive_compressed_result.push_back(static_cast<uint8_t>(data_size & 0xFF));
    adaptive_compressed_result.push_back(static_cast<uint8_t>((data_size >> 8) & 0xFF)); 
    adaptive_compressed_result.push_back(bitlens_count);
    for (int i = 0; i < bitlens.size(); i++) {
        adaptive_compressed_result.push_back(bitlens[i]);
    }
    for (int i = 0; i < symbols.size(); i++) {
        uint8_t symbol_value = symbols[i].get_value();
        adaptive_compressed_result.push_back(symbol_value);
    }
    for (int i = 0; i < compressed_input_bytes.size(); i++) {
        adaptive_compressed_result.push_back(compressed_input_bytes[i]);
    }
}


/*
* Tato funkce provede zakódování vstupního souboru.
* Vstupem je vektor použitých symbolů a nezakomprimovaná data.
* Pro každý byte vstupních dat se vyhledá jeho kanonický kód a ten se uloží do výstupního vektoru.
*/
vector<bool> compress_input(vector<Symbol>& symbols, vector<uint8_t>& input)
{
    vector<bool> compressed_input;

    for (int i = 0; i < input.size(); i++) {
        for (int j = 0; j < symbols.size(); j++) {
            if (input[i] == symbols[j].get_value()) {
                vector<bool> code = symbols[j].get_code();
                for (int k = 0; k < code.size(); k++) {
                    compressed_input.push_back(code[k]);
                }
                break;
            }
        }
    }

    return compressed_input;
}


/*
* Tato funkce provádí kompresi při adaptivním průchodu.
*
* Nejprve je provedena komprimace bloku ve vertikálním směru, tedy bloku, který je transponován.
* Dále je provedena komprimace bloku v horizontálním směru.
* Ten směr, který má lepší kompresní poměr je vybrán a výsledek je uložen.
*/
void adaptive_compress(vector<uint8_t>& vertical, vector<uint8_t>& horizontal)
{
    /*
    * Vertikální komprese
    */
    vector<int> frequencies_vertical = get_frequencies(vertical);
    vector<Symbol> symbols_vertical = create_symbols(frequencies_vertical);

    vector<int> bitlens_vertical = count_bitlens(symbols_vertical);

    for (int i = 0; i < bitlens_vertical.size(); i++) {
        symbols_vertical[i].set_bitlen(bitlens_vertical[i]);
    }

    sort(symbols_vertical.begin(), symbols_vertical.end(), [](Symbol a, Symbol b) {
        if (a.get_bitlen() == b.get_bitlen()) {
            return a.get_value() < b.get_value();
        } else {
            return a.get_bitlen() < b.get_bitlen();
        }
    });

    create_canonical_codes(symbols_vertical);

    uint8_t header_bitlens_count_vertical = count_unique_bitlens(symbols_vertical);
    vector<uint8_t> header_bitlens_vertical = count_bitlens_for_symbols(symbols_vertical, header_bitlens_count_vertical);
    vector<bool> compressed_input_vertical = compress_input(symbols_vertical, vertical);

    /***************************************************************************************/

    /*
    * Horizontální komprese
    */
    vector<int> frequencies_horizontal = get_frequencies(horizontal);
    vector<Symbol> symbols_horizontal = create_symbols(frequencies_horizontal);

    vector<int> bitlens_horizontal = count_bitlens(symbols_horizontal);

    for (int i = 0; i < bitlens_horizontal.size(); i++) {
        symbols_horizontal[i].set_bitlen(bitlens_horizontal[i]);
    }

    sort(symbols_horizontal.begin(), symbols_horizontal.end(), [](Symbol a, Symbol b) {
        if (a.get_bitlen() == b.get_bitlen()) {
            return a.get_value() < b.get_value();
        } else {
            return a.get_bitlen() < b.get_bitlen();
        }
    });

    create_canonical_codes(symbols_horizontal);

    uint8_t header_bitlens_count_horizontal = count_unique_bitlens(symbols_horizontal);
    vector<uint8_t> header_bitlens_horizontal = count_bitlens_for_symbols(symbols_horizontal, header_bitlens_count_horizontal);
    vector<bool> compressed_input_horizontal = compress_input(symbols_horizontal, horizontal);

    /***************************************************************************************/

    if (compressed_input_vertical.size() < compressed_input_horizontal.size()) {
        add_to_output(0, header_bitlens_count_vertical, header_bitlens_vertical, symbols_vertical, compressed_input_vertical);
    } else {
        add_to_output(1, header_bitlens_count_horizontal, header_bitlens_horizontal, symbols_horizontal, compressed_input_horizontal);
    }
}


/*
* Tato funkce provede kompresi vstupního souboru při statickém průchodu.
* Při statickém průchodu není potřeba využívat globálních vektorů a výsledek je tak přímo uložen do výstupního souboru.
*/
void compress(vector<uint8_t>& input, string output_filename)
{
    vector<int> frequencies = get_frequencies(input);
    vector<Symbol> symbols = create_symbols(frequencies);

    vector<int> bitlens = count_bitlens(symbols);

    for (int i = 0; i < bitlens.size(); i++) {
        symbols[i].set_bitlen(bitlens[i]);
    }

    sort(symbols.begin(), symbols.end(), [](Symbol a, Symbol b) {
        if (a.get_bitlen() == b.get_bitlen()) {
            return a.get_value() < b.get_value();
        } else {
            return a.get_bitlen() < b.get_bitlen();
        }
    });

    create_canonical_codes(symbols);

    uint8_t header_bitlens_count = count_unique_bitlens(symbols);
    vector<uint8_t> header_bitlens = count_bitlens_for_symbols(symbols, header_bitlens_count);
    vector<bool> compressed_input = compress_input(symbols, input);

    write_output(output_filename, header_bitlens_count, header_bitlens, symbols, compressed_input);
}


/*
* Tato funkce provádí dekompresi vstupního souboru při adaptivním přístupu.
*
* Nejprve je zpracována hlavička a získány informace o aktuálně dekomprimovaném bloku. Datová část bloku
* je následně dekomprimována dle algoritmu z přednášek.
* Blok, který byl zakomprimován ve vertikálním směru musí být zpětně transponován. K dopočítání šířky a výšky
* takového bloku slouží pomocná třída SubmatrixInfo.
*/
vector<uint8_t> adaptive_decompress(vector<uint8_t>& input, string output_filename, bool is_model, uint16_t image_width, uint16_t image_height)
{
    int last = input.size();
    int i = 0;
    vector<uint8_t> decompressed_input;
    vector<uint8_t> decompressed_input_result;
    int prev_tmp_size = 0;
    SubmatrixInfo submatrix_info(image_height, image_width);

    while (i != last) {
        uint8_t padding = input[i];
        uint8_t direction = input[i + 1];
        uint8_t lower_byte_data_counts = input[i + 2];
        uint8_t upper_byte_data_counts = input[i + 3];
        uint16_t data_counts = (static_cast<uint16_t>(upper_byte_data_counts) << 8) | lower_byte_data_counts;
        uint8_t header_bitlens_count = input[i + 4];

        vector<int> header_bitlens;
        header_bitlens.push_back(0);

        for (int j = 0; j < header_bitlens_count; j++) {
            header_bitlens.push_back(input[i + j + 5]);
        }

        if (header_bitlens[header_bitlens.size() - 1] == 0) { 
            header_bitlens[header_bitlens.size() - 1] = 256;
        }

        int c = 0;
        int s = 1;

        vector<int> first_symbol(header_bitlens.size() + 1);
        vector<int> first_code(header_bitlens.size() + 1);

        for (int j = 1; j < header_bitlens.size() + 1; j++) {
            first_code[j] = c;
            first_symbol[j] = s;

            s = s + header_bitlens[j];
            c = (c + (header_bitlens[j] - 1) + 1) << 1;
        }

        int alphabet_count = 0;
        for (int j = 0; j < header_bitlens.size(); j++) {
            alphabet_count += header_bitlens[j];
        }

        int alphabet_start = header_bitlens_count + 5 + i;
        int alphabet_end = alphabet_start + alphabet_count;

        vector<uint8_t> alphabet;
        for (int j = alphabet_start; j < alphabet_end; j++) {
            alphabet.push_back(input[j]);
        }

        vector<bool> input_as_bits;
        for (int j = alphabet_end; j < (alphabet_end + data_counts); j++) {
            for (int k = 0; k < 8; k++) {
                input_as_bits.push_back(input[j] & (1 << (7 - k)));
            }
        }

        int cd = 0;
        int ld = 0;

        for (int i = 0; i < input_as_bits.size() - padding; i++) {
            ld = ld + 1;
            if (input_as_bits[i]) {
                cd = (cd << 1) + 1;
            } else {
                cd = (cd << 1) + 0;
            }

            if ((cd << 1) < first_code[ld + 1]) {
                int index = first_symbol[ld] + cd - first_code[ld];
                decompressed_input.push_back(alphabet[index - 1]);
                cd = 0;
                ld = 0;
            }
        }

        int tmp_size = decompressed_input.size();
        vector<uint8_t> tmp_decompressed_input(decompressed_input.begin() + prev_tmp_size, decompressed_input.begin() + tmp_size);
        if (is_model) {
            tmp_decompressed_input = set_decompress_model(tmp_decompressed_input);
        }

        prev_tmp_size = tmp_size;

        int next_height = submatrix_info.get_next_height();
        int next_width = submatrix_info.get_next_width();

        if (direction == 1) {
            tmp_decompressed_input = transpose_matrix(tmp_decompressed_input, next_width, next_height);
        }

        submatrix_info.move_to_next();

        for (int j = 0; j < tmp_decompressed_input.size(); j++) {
            decompressed_input_result.push_back(tmp_decompressed_input[j]);
        }

        i = i + 5 + header_bitlens_count + alphabet_count + data_counts;

    }

    return decompressed_input_result;

}


/*
* Tato funkce provádí dekompresi vstupního souboru při statickém přístupu.
*
* Nejprve je zpracována hlavička souboru, která obsahuje informace o zakomprimovaných datech.
* Dekomprimace dále probíhá dle algoritmu z přednášky.
*/
vector<uint8_t> decompress(vector<uint8_t>& input, string output_filename)
{
    uint8_t padding = input[0];
    uint8_t header_bitlens_count = input[1];

    vector<int> header_bitlens;
    header_bitlens.push_back(0);
    for (int i = 0; i < header_bitlens_count; i++) {
        header_bitlens.push_back(input[i + 2]);
    }

    if (header_bitlens[header_bitlens.size() - 1] == 0) { 
        header_bitlens[header_bitlens.size() - 1] = 256;
    }

    int c = 0;
    int s = 1;

    vector<int> first_symbol(header_bitlens.size() + 1);
    vector<int> first_code(header_bitlens.size() + 1);

    for (int i = 1; i < header_bitlens.size() + 1; i++) {
        first_code[i] = c;
        first_symbol[i] = s;

        s = s + header_bitlens[i];
        c = (c + (header_bitlens[i] - 1) + 1) << 1;
    }

    int alphabet_count = 0;
    for (int i = 0; i < header_bitlens.size(); i++) {
        alphabet_count += header_bitlens[i];
    }


    int alphabet_start = header_bitlens_count + 2;
    int alphabet_end = alphabet_start + alphabet_count;

    vector<uint8_t> alphabet;
    for (int i = alphabet_start; i < alphabet_end; i++) {
        alphabet.push_back(input[i]);
    }

    vector<bool> input_as_bits;
    for (int i = alphabet_end; i < input.size(); i++) {
        for (int j = 0; j < 8; j++) {
            input_as_bits.push_back(input[i] & (1 << (7 - j)));
        }
    }

    int cd = 0;
    int ld = 0;

    vector<uint8_t> decompressed_input;

    for (int i = 0; i < input_as_bits.size() - padding; i++) {
        ld = ld + 1;
        if (input_as_bits[i]) {
            cd = (cd << 1) + 1;
        } else {
            cd = (cd << 1) + 0;
        }
        
        if ((cd << 1) < first_code[ld + 1]) {
            int index = first_symbol[ld] + cd - first_code[ld];
            decompressed_input.push_back(alphabet[index - 1]);
            cd = 0;
            ld = 0;
        }
    }

    return decompressed_input;
}
