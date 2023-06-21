/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: input.cpp
 * Description: Contains functions to process input arguments and print help.
 */

#include "input.h"
#include "errors.h"

using namespace std;

bool isVerboseActivated = false;


void showHelp()
{
    cout << "ISA-BOT NAPOVEDA:" << endl;
    cout << "Povolene argumenty ke spousteni:" << endl;
    cout << "[-h|-help] - Pro vypis napovedy" << endl;
    cout << "[-v|-verbose] - Pro vypis zprav na ktere BOT reaguje na STDOUT" << endl;
    cout << "-t <bot_access_token> - Pro zadani pristupoveho tokenu BOTa. Tento argument je povinny" << endl;
    exit(0);
}


string parseArguments(int argc, char* argv[])
{
    bool helpArgumentAlreadyEntered = false;
    bool verboseArgumentAlreadyEntered = false;
    bool tokenArgumentAlreadyEntered = false;

    string botToken;

    if (argc > 5) {
        errorCall("Too many arguments! Try -h or -help", INPUT_ARGUMENTS_ERROR);
    }
    else if (argc < 2) {
        errorCall("Too less arguments! Try -h or -help", INPUT_ARGUMENTS_ERROR);
    }

    for (int x = 1; x < argc; x++) {
        if ((string(argv[x]) == "-h" || string(argv[x]) == "--help") && helpArgumentAlreadyEntered == false) {
            helpArgumentAlreadyEntered = true;
        }
        else if ((string(argv[x]) == "-v" || string(argv[x]) == "--verbose") && verboseArgumentAlreadyEntered == false) {
            verboseArgumentAlreadyEntered = true;
            isVerboseActivated = true;
        }
        else if (string(argv[x]) == "-t" && tokenArgumentAlreadyEntered == false) {
            tokenArgumentAlreadyEntered = true;
            if (argv[x+1] != NULL) {
                botToken = string(argv[x+1]);
            }
            else {
                errorCall("Mising BOT token", INPUT_ARGUMENTS_ERROR);
            }
        }
        else if (string(argv[x-1]) == "-t") {
            continue;
        }
        else {
            errorCall("Wrong arguments!", INPUT_ARGUMENTS_ERROR);
        }
    }

    if (helpArgumentAlreadyEntered == true) {
        showHelp();
    }

    if(botToken == "")
    {
        errorCall("Mising BOT token", INPUT_ARGUMENTS_ERROR);
    }
    
    return botToken;
}