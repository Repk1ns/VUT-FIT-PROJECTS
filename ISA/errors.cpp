/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: errors.cpp
 * Description: Contains error function which is called in case of error. 
 * It exit program with specific error code.
 */

#include "errors.h"

using namespace std;

void errorCall(string errorMessage, int errorCode)
{
    cout << "ERROR: " << errorCode << " : " << errorMessage << endl;
    exit(errorCode);
}