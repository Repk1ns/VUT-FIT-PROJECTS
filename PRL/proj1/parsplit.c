#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>

using namespace std;

/**
*
* Funkce readInput() načte vstupní soubor numbers a uložít jej do vektoru.
* Pokud se soubor nepodaří otevřít, vypíše chybovou hlášku a ukončí program.
*
*/
vector<uint8_t> readInput()
{
    vector<uint8_t> input;
    uint8_t number;
    ifstream inputFile;

    inputFile.open("numbers", ios::in | ios::binary);

    if ( ! inputFile.is_open()) {
        cerr << "Vstupni soubor se nepodarilo otevrit" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);

        return input;
    }

    while (inputFile.read(reinterpret_cast<char*>(&number), sizeof(number))) {
        input.push_back(number);
    }

    return input;
}

/**
*
* Pomocná funkce pro výpis vektoru čísel.
*
*/
void printNumbers(vector<uint8_t> numbers)
{
    for (uint8_t i: numbers) {
        cout << unsigned(i) << " ";
    }
    cout << endl;
}

/**
*
* Funkce getMid() vrací průměr dvou čísel v polovině vektoru.
*
*/
uint8_t getMid(vector<uint8_t> numbers)
{
    int size = numbers.size();
    if (size % 2 == 0) {
        return numbers[size / 2 - 1];
    }

    return numbers[size / 2];
}


int main(int argc, char** argv)
{
    int rank;
    int size;
    vector<uint8_t> numbers;
    uint8_t mid;
    int chunk_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        numbers = readInput();

        //
        // Ošetření vstupních dat - počet čísel musí být v rozmezí 8-64
        //
        if (numbers.size() < 8 || numbers.size() > 64) {
            cerr << "ERROR: Too much or too less input numbers. (Allowerd range: 8-64)" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        mid = getMid(numbers);
        //printNumbers(numbers);
        //cout << "MID: " << unsigned(mid) << endl;

        //
        // Ošetření vstupních dat - počet čísel musí být dělitelný počtem procesů beze zbytku
        //
        if (numbers.size() % size != 0) {
            cerr << "ERROR: Input numbers count must be exactly divisible by the number of processes." << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        chunk_size = numbers.size() / size;
    }

    //
    // Broadcast hodnoty mid a chunk_size na všechny procesy
    //
    MPI_Bcast(&mid, 1, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<uint8_t> local_numbers(chunk_size);

    //
    // Rozdělení vektoru numbers na všechny procesy
    //
    MPI_Scatter(numbers.data(), chunk_size, MPI_UINT8_T, local_numbers.data(), chunk_size, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    //
    // Výpočet počtu čísel menších, rovných a větších než mid lokálně na všech procesech.
    //
    vector<uint8_t> less, equal, greater;
    for (int i = 0; i < local_numbers.size(); i++) {
        if (local_numbers[i] < mid) {
            less.push_back(local_numbers[i]);
        } else if (local_numbers[i] == mid) {
            equal.push_back(local_numbers[i]);
        } else if (local_numbers[i] > mid) {
            greater.push_back(local_numbers[i]);
        }
    }

    int *less_displs = (int*)malloc(size * sizeof(int));
    int *less_recvcounts = (int*)malloc(size * sizeof(int));

    int *equal_displs = (int*)malloc(size * sizeof(int));
    int *equal_recvcounts = (int*)malloc(size * sizeof(int));

    int *greater_displs = (int*)malloc(size * sizeof(int));
    int *greater_recvcounts = (int*)malloc(size * sizeof(int));

    int less_size = less.size();
    int equal_size = equal.size();
    int greater_size = greater.size();

    //
    // Vypočítání sumy prefixů - displacements
    //
    MPI_Exscan(&less_size, &less_displs[rank], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Exscan(&equal_size, &equal_displs[rank], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Exscan(&greater_size, &greater_displs[rank], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    less_displs[0] = 0;
    equal_displs[0] = 0;
    greater_displs[0] = 0;

    //
    // Dopočítání velikostí lokálních polí s výsledky - counts
    //
    less_recvcounts[0] = less_size;
    for (int i = 1; i < size; i++) {
        less_recvcounts[i] = less_size;
    }

    equal_recvcounts[0] = equal_size;
    for (int i = 1; i < size; i++) {
        equal_recvcounts[i] = equal_size;
    }

    greater_recvcounts[0] = greater_size;
    for (int i = 1; i < size; i++) {
        greater_recvcounts[i] = greater_size;
    }

    int *root_less_displacements = NULL;
    int *root_less_recvcounts = NULL;

    int *root_equal_displacements = NULL;
    int *root_equal_recvcounts = NULL;

    int *root_greater_displacements = NULL;
    int *root_greater_recvcounts = NULL;

    if (rank == 0) {
        root_less_displacements = (int *) malloc(sizeof(int) * size);
        root_less_recvcounts = (int *) malloc(sizeof(int) * size);

        root_equal_displacements = (int *) malloc(sizeof(int) * size);
        root_equal_recvcounts = (int *) malloc(sizeof(int) * size);
        
        root_greater_displacements = (int *) malloc(sizeof(int) * size);
        root_greater_recvcounts = (int *) malloc(sizeof(int) * size);
    }

    //
    // Sběr sumy prefixů a velikostí na root proces
    //
    MPI_Gather(&less_displs[rank], 1, MPI_INT, root_less_displacements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&less_recvcounts[rank], 1, MPI_INT, root_less_recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gather(&equal_displs[rank], 1, MPI_INT, root_equal_displacements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&equal_recvcounts[rank], 1, MPI_INT, root_equal_recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Gather(&greater_displs[rank], 1, MPI_INT, root_greater_displacements, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&greater_recvcounts[rank], 1, MPI_INT, root_greater_recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int less_total_length = 0;
    int equal_total_length = 0;
    int greater_total_length = 0;

    //
    // Vypočítání celkové délky výsledných polí na root procesu
    //
    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            less_total_length += root_less_recvcounts[i];
        }

        for (int i = 0; i < size; i++) {
            equal_total_length += root_equal_recvcounts[i];
        }

        for (int i = 0; i < size; i++) {
            greater_total_length += root_greater_recvcounts[i];
        }
    }
    
    uint8_t *less_recvbuf = NULL;
    less_recvbuf = (uint8_t*)malloc(less_total_length * sizeof(uint8_t));

    uint8_t *equal_recvbuf = NULL;
    equal_recvbuf = (uint8_t*)malloc(equal_total_length * sizeof(uint8_t));

    uint8_t *greater_recvbuf = NULL;
    greater_recvbuf = (uint8_t*)malloc(greater_total_length * sizeof(uint8_t));

    //
    // Sběr výsledků na root proces
    //
    MPI_Gatherv(less.data(), less_size, MPI_UINT8_T, less_recvbuf, root_less_recvcounts, root_less_displacements, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gatherv(equal.data(), equal_size, MPI_UINT8_T, equal_recvbuf, root_equal_recvcounts, root_equal_displacements, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    MPI_Gatherv(greater.data(), greater_size, MPI_UINT8_T, greater_recvbuf, root_greater_recvcounts, root_greater_displacements, MPI_UINT8_T, 0, MPI_COMM_WORLD);

    //
    // Výpis výsledků na STDOUT
    //
    if (rank == 0) {
        printf("L: ");
        for (int i = 0; i < less_total_length; i++) {
            printf("%d ", less_recvbuf[i]);
        }
        printf("\n");
        printf("E: ");
        for (int i = 0; i < equal_total_length; i++) {
            printf("%d ", equal_recvbuf[i]);
        }
        printf("\n");
        printf("G: ");
        for (int i = 0; i < greater_total_length; i++) {
            printf("%d ", greater_recvbuf[i]);
        }
        printf("\n");
    }

    MPI_Finalize();

    return 0;
}
