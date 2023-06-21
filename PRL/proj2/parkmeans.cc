#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <typeinfo>
#include <algorithm>

using namespace std;

/**
*
* Funkce read_input() načte vstupní soubor numbers a uložít jej do vektoru.
* Pokud se soubor nepodaří otevřít, vypíše chybovou hlášku a ukončí program.
*
*/
vector<uint8_t> read_input()
{
    vector<uint8_t> input;
    uint8_t number;
    ifstream input_file;

    input_file.open("numbers", ios::in | ios::binary);

    if ( ! input_file.is_open()) {
        cerr << "Vstupni soubor se nepodarilo otevrit" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);

        return input;
    }

    while (input_file.read(reinterpret_cast<char*>(&number), sizeof(number))) {
        input.push_back(number);
    }

    return input;
}


/*
* Pomocná funkce pro výpis obsahu vektoru
*/
void print_vector(vector<int>& vec)
{
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i != vec.size() - 1) {
            cout << ", ";
        }
    }
    cout << endl;
}


int main(int argc, char** argv)
{
    int rank;
    int size;
    vector<uint8_t> numbers;
    vector<double> means(4);
    vector<int> cluster_counts(4);
    vector<int> cluster_sums(4);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        numbers = read_input();

        /*
        * Ošetření vstupních dat: 
        * - Počet čísel musí být v rozmezí 4-32
        * - Počet procesorů musí být optimálně stejný nebo větší než počet vstupních čísel
        * - Pokud je čísel více než 4, ale procesorů méně než 4, vypíše se chyba.
        *   Je-li procesorů méně než čísel, musí jich být alespoň 4.
        *   V opačném případě by se vektor čísel zkrátil na počet procesorů, čímž pádem
        *   by nebyl splněn rozsah čísel 4-32.
        */
        if (numbers.size() < 4 || numbers.size() > 32) {
            cerr << "ERROR: Too much or too few input numbers. (Allowed range: 4-32)" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (numbers.size() < size) {
            cerr << "ERROR: Too much processes. (There must be at least as many numbers as processes)" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (size < 4) {
            cerr << "ERROR: Too few processes. (There must be at least 4 processes)" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (numbers.size() > size) {
            numbers.resize(size);
        }

        for (int i = 0; i < 4; i++) {
            means[i] = numbers[i];
        }

    }

    /*
    * Inicialiazce lokálních proměnných pro každý procesor
    */
    uint8_t local_number;
    vector<int> root_cluster_counts(4);
    vector<int> root_cluster_sums(4);

    /* 
    * Jelikož dopředně nevíme, kolik v kterém clusteru bude ve výsledku čísel, inicializujeme výsledky na hodnoty -1
    * které v průběhu nahradíme. Zbylé hodnoty -1 budou na konci odstraněny.
    */
    int local_first = -1;
    int local_second = -1;
    int local_third = -1;
    int local_fourth = -1;

    /*
    * Inicializace polí pro výsledky
    */
    vector<int> result_first(size);
    vector<int> result_second(size);
    vector<int> result_third(size);
    vector<int> result_fourth(size);
    
    /*
    * Rozeslání čísel každému procesoru
    * Hlavní smyčka do-while
    */
    MPI_Scatter(numbers.data(), 1, MPI_UINT8_T, &local_number, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    int old_index = -1;
    bool flag;
    bool root_flag;
    do {
        MPI_Bcast(means.data(), means.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
        cluster_counts = {0, 0, 0, 0};
        cluster_sums = {0, 0, 0, 0};
        local_first = -1;
        local_second = -1;
        local_third = -1;
        local_fourth = -1;
        
        // Výpočet vzdáleností
        double tmp_distance = abs((double)local_number - means[0]);
        int index = 0;
        for (int i = 1; i < means.size(); i++) {
            double distance = abs((double)local_number - means[i]);

            if (distance <= tmp_distance) {
                tmp_distance = distance;
                index = i;
            }
        }

        // Naplnění redukovaných polí
        cluster_counts[index]++;
        cluster_sums[index] = local_number;

        flag = (old_index != index);
        old_index = index;

        // Uložení lokálních výsledků
        switch (index) {
            case 0:
                local_first = local_number;
                break;
            case 1:
                local_second = local_number;
                break;
            case 2:
                local_third = local_number;
                break;
            case 3:
                local_fourth = local_number;
                break;
        }

        MPI_Reduce(cluster_counts.data(), root_cluster_counts.data(), cluster_counts.size(), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(cluster_sums.data(), root_cluster_sums.data(), cluster_sums.size(), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&flag, &root_flag, 1, MPI_CXX_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
        
        // Vypočítání nových středů z redukovaných hodnot
        if (rank == 0) {
            for (int i = 0; i < 4; i++) {
                means[i] = (double)root_cluster_sums[i] / (double)root_cluster_counts[i];
            }
        }

        // Rozeslání informace pro všechny procesy o tom, zdali má cyklus pokračovat
        MPI_Bcast(&root_flag, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);

    } while(root_flag);

    /*
    * Stažení výsledků do ROOT procesu
    */
    MPI_Gather(&local_first, 1, MPI_INT, result_first.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_second, 1, MPI_INT, result_second.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_third, 1, MPI_INT, result_third.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_fourth, 1, MPI_INT, result_fourth.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*
    * Úprava vektorů s výsledky - odstranění -1 hodnot
    */
    result_first.erase(remove(result_first.begin(), result_first.end(), -1), result_first.end());
    result_second.erase(remove(result_second.begin(), result_second.end(), -1), result_second.end());
    result_third.erase(remove(result_third.begin(), result_third.end(), -1), result_third.end());
    result_fourth.erase(remove(result_fourth.begin(), result_fourth.end(), -1), result_fourth.end());

    /*
    * Výpis výsledků na STDOUT
    */
    if(rank == 0) {
        cout << "[" << means[0] << "] ";
        print_vector(result_first);
        cout << "[" << means[1] << "] "; 
        print_vector(result_second);
        cout << "[" << means[2] << "] ";
        print_vector(result_third);
        cout << "[" << means[3] << "] ";
        print_vector(result_fourth);
    }

    MPI_Finalize();

    return 0;
}