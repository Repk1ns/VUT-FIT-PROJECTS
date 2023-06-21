/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: errors.h
 * Description: Header file.
 */

#ifndef ERRORS_H
#define ERRORS_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

/**
 * CONSTANTS - ERROR CODES
 */
#define INPUT_ARGUMENTS_ERROR -1
#define HOST_ERROR -2
#define SOCKET_ERROR -3
#define SSL_ERROR -4
#define DISCORD_ERROR -5


/**
 * Function that stops program with specific error code and prints error message to STDOUT.
 */
void errorCall(std::string errorMessage, int errorCode);

#endif