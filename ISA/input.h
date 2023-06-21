/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: input.h
 * Description: Header file.
 */

#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

extern bool isVerboseActivated;

/**
 * Show help in case of -h|-help input argument and ends program.
 */
void showHelp();

/**
 * Function to process given input arguments. 
 * It returns given Bot Token and sets global -v|-verbose property.
 */
std::string parseArguments(int argc, char* argv[]);

#endif