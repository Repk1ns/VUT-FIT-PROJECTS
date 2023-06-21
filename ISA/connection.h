/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: connection.h
 * Description: Header file.
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <sys/socket.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

/**
 * CONSTANTS
 */
#define ADDRESS "discord.com"
#define PORT 443
#define BUFFER 4096

struct connection
{
    SSL* ssl;
    SSL_CTX *ctx;
    int sock = 0;
};

/**
 * Function that initializes SSL structure, CTX structure, socket, gets host address and starts a connection with Discord.
 * It returns structure connection.
 */
connection initConnection();

/**
 * Release allocation of SSL, Socket and CTX.
 */
void clearConnection(connection conn);

/**
 * Function that sends requests to Discord and read responses.
 * It returns HTTP response from Discord.
 */
std::string sendRequest(SSL *ssl, int requestType, const std::string &botToken, const std::string &guildId = "", const std::string &channelId = "", const std::string &lastMessageId = "", const std::string &content = "");

#endif