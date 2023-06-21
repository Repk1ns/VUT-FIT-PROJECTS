/**
 * Fakulta informačních technologií VUT v Brně
 * Předmět: IMS 2020/2021
 * Autoři: Vojtěch Mimochodek (xmimoc01), Martin Berger (xberge04)
 * Datum: 7.12.2020
 */


#include <cstdlib>
#include <simlib.h>
#include <getopt.h>
#include <iostream>
#include "main.hpp"
#include <tgmath.h>

using namespace std;

Facility bakerPause("BakerPause");
Facility furnace("Furnace");

bool endOfShift = false;
int couriers = 0;
int orders = 0;
int shift = 0;
int furnacesCapacity = 0;

double input = 0.0;

double orderProcessTime = 0.0;
double processEndTime = 0.0;
double ordersPreparingTime = 0.0;
int processedOrders = 0;
int ordersQueue = 0;
int ordersWaitingToDelivery = 0;
int allOrders = 0;
int acceptedOrders = 0;
int refusedOrders = 0;


/**
 * Funkce pro vypsání nápovědy
 */
void showHelp()
{
    cout << "Nápověda:" << endl;
    cout << "Povolené argumenty ke spuštění:\n-o pro relativní počet objednávek" << endl;
    cout << "-c pro zadání počtu kurýrů" << endl;
    cout << "-s pro délku směny" << endl;
    cout << "-h pro vypsání nápovědy" << endl;
    cout << "-f pro kapacitu současně pečených pizz" << endl;

    exit(0);
}


/**
 * Proces pro schvalování či zamítání objednávek.
 */
class OrderGenerator : public Process 
{
    void Behavior() 
    {
        allOrders++;
        if (bakerPause.Busy() == false && furnace.Busy() == false) {

            if (shift - Time > ceil(((double)ordersQueue + 1) / furnacesCapacity) * (furnacesCapacity * 2 + BAKING_TIME)) {
                if (Time < shift - 30) {
                    ordersQueue += 1;
                    acceptedOrders += 1;
                }
                else
                {
                    refusedOrders += 1;   
                }
            }
            else
            {
                refusedOrders += 1;   
            }
            
        }
        else
        {
            if (shift - processEndTime > ceil(((double)ordersQueue + 1) / furnacesCapacity) * (furnacesCapacity * 2 + BAKING_TIME)) {
                if (Time < shift - 30) {
                    ordersQueue += 1;
                    acceptedOrders += 1;
                }
                else
                {
                    refusedOrders += 1;   
                }
            }
            else
            {
                refusedOrders += 1;   
            }
        }
    }
};


/**
 * Proces simulující pečení pizzy a předávání zpracovaných objednávek kurýrům.
 */
class OrdersCheck : public Process
{
    void Behavior() 
    {
        if (bakerPause.Busy() == false && furnace.Busy() == false) {

            if (ordersQueue < 3) {

                Seize(bakerPause);
                processEndTime = Time + BAKER_PAUSE_TIME;
                Wait(BAKER_PAUSE_TIME);
                Release(bakerPause);

                if (ordersQueue > 0) {

                    if (ordersQueue < furnacesCapacity) {

                        int ordersInProcess = ordersQueue;
                        ordersQueue = 0;
                        Seize(furnace);
                        ordersPreparingTime = SINGLE_PIZZA_PREPARING_TIME * ordersInProcess + BAKING_TIME;
                        processEndTime = Time + ordersPreparingTime;
                        Wait(ordersPreparingTime);
                        Release(furnace);
                        ordersWaitingToDelivery += ordersInProcess;
                    }

                    if (ordersQueue >= furnacesCapacity) {

                        ordersQueue -= furnacesCapacity;
                        Seize(furnace);
                        ordersPreparingTime = SINGLE_PIZZA_PREPARING_TIME * furnacesCapacity + BAKING_TIME;
                        processEndTime = Time + ordersPreparingTime;
                        Wait(ordersPreparingTime);
                        Release(furnace);
                        ordersWaitingToDelivery += furnacesCapacity;
                    }
                }
            }
            else {

                if (ordersQueue < furnacesCapacity) {

                    int ordersInProcess = ordersQueue;
                    ordersQueue = 0;
                    Seize(furnace);
                    ordersPreparingTime = SINGLE_PIZZA_PREPARING_TIME * furnacesCapacity + BAKING_TIME;
                    processEndTime = Time + ordersPreparingTime;
                    Wait(ordersPreparingTime);
                    Release(furnace);
                    ordersWaitingToDelivery += ordersInProcess;
                }
                if (ordersQueue >= furnacesCapacity) {

                    ordersQueue -= furnacesCapacity;
                    Seize(furnace);
                    ordersPreparingTime = SINGLE_PIZZA_PREPARING_TIME * furnacesCapacity + BAKING_TIME;
                    processEndTime = Time + ordersPreparingTime;
                    Wait(ordersPreparingTime);
                    Release(furnace);
                    ordersWaitingToDelivery += furnacesCapacity;
                }
            }
        }
    }
};


/**
 * Proces simulující rozvoz kurýrů s počítadlem úspěšně dokončených objednávek.
 */
class CourierGenerator : public Process
{
    void Behavior()
    {
        int tookOrders = 0;
        int kilometers = 0;
        if (ordersWaitingToDelivery > 0) {
            if (couriers != 0) {

                if (ordersWaitingToDelivery >= 4) {
                    tookOrders = 4;
                    ordersWaitingToDelivery -= 4;
                }
                else if (ordersWaitingToDelivery < 4) {
                    tookOrders = ordersWaitingToDelivery;
                    ordersWaitingToDelivery -= tookOrders;
                }

                for (int i = 0; i < tookOrders; i++) {
                    kilometers += rand() % 10 + 1;
                }

                double deliveryTime = ((double)kilometers / 35) * 60;

                couriers -= 1;
                Wait(deliveryTime);
                processedOrders += tookOrders;
                couriers += 1;
            }
        }
    }
};


/**
 * Hlavní generátor pro vytváření objednávek.
 */
class ShiftOrders : public Event
{
    void Behavior()
    {
        if (Time < shift) {
            (new OrderGenerator)->Activate();
            Activate(Time + Exponential(input));
        }
    }
};


/**
 * Hlavní generátor pro kuchyň - uvedení kuchařů do chodu.
 */
class ShiftBaking : public Event
{
    void Behavior()
    {
        if (Time < shift) {
            (new OrdersCheck)->Activate();
            Activate(Time + 1);
        }
        
    }
};


/**
 * Hlavní generátor pro rozvoz - uvedení kurýrů do chodu.
 */
class ShiftCouriers : public Event
{
    void Behavior()
    {
        if (Time < shift + 70) {
            (new CourierGenerator)->Activate();
            Activate(Time + 1);
        }
    }
};


/**
 * Hlavní tělo programu.
 */
int main(int argc, char *argv[])
{
    int arg;
    string type;

    /**
     * Zpracování vstupní argumentů.
     */
    while ((arg = getopt(argc, argv, "o:c:t:s:hf:")) != -1)
    {
        switch(arg)
        {
            case 'o':
                orders = atoi(optarg);
                if (orders == 0) {
                    cout << "Error in argument -o: Value is not a number" << endl;
                    exit(-1);
                }
                break;
            
            case 'c':
                couriers = atoi(optarg);
                if (couriers == 0) {
                    cout << "Error in argument -c: Value is not a number" << endl;
                    exit(-1);
                }
                break;
            
            case 's':
                shift = atoi(optarg);
                if (shift == 0)
                {
                    cout << "Error in argument -s: Value is not a number" << endl;
                    exit(-1);
                }

                if (shift < 1 || shift > 24) {
                    cout << "Error in argument -s: Shift can be between 1 - 24 hours" << endl;
                    exit(-1);
                }
                break;
            
            case 'f':
                furnacesCapacity = atoi(optarg);
                if (furnacesCapacity == 0)
                {
                    cout << "Error in argument -s: Value is not a number" << endl;
                    exit(-1);
                }

                break;
            
            case 'h':
                showHelp();
                break;
            
            case '?':
            default:
                cout << "Error. Try rerun with -h parameter" << endl;
                exit(-1);
        }
    }

    if (orders == 0) {
        cout << "Error: Parameter -o is required" << endl;
        exit(-1);
    }
    else if (couriers == 0) {
        cout << "Error: Parameter -c is required" << endl;
        exit(-1);
    }
    else if (shift == 0) {
        shift = 12;
    }
    else if (furnacesCapacity == 0) {
        cout << "Error: Parameter -f is required" << endl;
        exit(-1);
    }

    /**
     * Délka směny v minutách.
     */
    shift = shift * 60;

    /**
     * Inicializace simulace.
     */
    Init(SIMULATION_START_TIME, shift + 100);
    input = (double)shift / (double)orders;

    /**
     * Aktivace hlavních generátorů.
     */
    auto shiftCouriers = (new ShiftCouriers);
    shiftCouriers->Activate();

    auto shiftOrders = (new ShiftOrders);
    shiftOrders->Activate();

    auto shiftBaking = (new ShiftBaking);
    shiftBaking->Activate();

    RandomSeed(time(NULL));

    Run();

    delete(shiftOrders);
    delete(shiftBaking);
    delete(shiftCouriers);

    /**
     * Výpis výsledků.
     */
    cout << "========================================================" << endl;
    cout << "======================= SIMULACE =======================" << endl;
    cout << "\n";
    cout << " Relativní počet objednávek: " << orders << endl;
    cout << " Počet kurýrů: " << couriers << endl;
    cout << " Kapacita pece: " << furnacesCapacity << endl;
    cout << " Délka směny: " << shift / 60 << " hodin" << endl;
    cout << "\n";
    cout << "========================================================" << endl;
    cout << "\n" << endl;
    cout << "========================================================" << endl;
    cout << "======================= VÝSLEDEK =======================" << endl;
    cout << "\n";
    cout << " Počet všech příchozích objednávek: " << allOrders << endl;
    cout << " Počet přijatých objednávek: " << acceptedOrders << endl;
    cout << " Počet odmítnutých objednávek: " << refusedOrders << endl;
    cout << " Počet zpracovaných objednávek: " << processedOrders << endl;
    cout << " Počet nezpracovaných objednávek: " << acceptedOrders - processedOrders << endl;
    cout << "\n";
    cout << "========================================================" << endl;

    return 0;

}
