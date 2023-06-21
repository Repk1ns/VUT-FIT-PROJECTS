/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: connection.cpp
 * Description: Contains functions to initialize Socket, SSL and starts connection.
 */

#include "connection.h"
#include "parse.h"
#include "requests.h"
#include "errors.h"

using namespace std;


connection initConnection()
{
    struct sockaddr_in server;
    struct connection conn;
    
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();

    conn.ctx = SSL_CTX_new(TLSv1_2_method());

    if((conn.sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        errorCall("Error in creating socket", SOCKET_ERROR);
    }

    memset(&server, 0, sizeof(server));

    struct hostent *host;
    host = gethostbyname(ADDRESS);

    if (host == NULL) {
        errorCall("Cannot get host from gethostbyname()!", HOST_ERROR);
    }

    string IP = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(IP.c_str());
    server.sin_port = htons(PORT);

    if (connect(conn.sock, (struct sockaddr *)&server, sizeof(server)) != 0) {
        errorCall("Error in socket connecting", SOCKET_ERROR);
    }
    
    conn.ssl = SSL_new(conn.ctx);

    if(!conn.ssl) {
        errorCall("Error in creating SSL", SSL_ERROR);
    }

    SSL_set_fd(conn.ssl, conn.sock);

    if(SSL_connect(conn.ssl) <= 0) {
        errorCall("Error in SSL connecting", SSL_ERROR);
    }

    return conn;
}


void clearConnection(connection conn)
{
    SSL_free(conn.ssl);
    SSL_CTX_free(conn.ctx);
    close(conn.sock);
}


string sendRequest(SSL *ssl, int requestType, const string &botToken, const string &guildId, const string &channelId, const string &lastMessageId, const string &content)
{
    const char *request;
    string strRequest;
    stringstream response;
    string isChunked;
    char buffer[BUFFER];

    if (requestType == GET_CLIENT_ID) {
        strRequest = createIDRequest(botToken);
    }
    else if (requestType == GET_GUILDS) {
        strRequest = createGuildsRequest(botToken);
    }
    else if (requestType == GET_CHANNELS) {
        strRequest = createChannelRequest(botToken, guildId);
    }
    else if (requestType == GET_MESSAGES) {
        strRequest = createMessagesRequest(botToken, channelId);
    }
    else if (requestType == GET_ACTUAL_MESSAGES) {
        strRequest = createActualMessagesRequest(botToken, channelId, lastMessageId);
    }
    else if(requestType == POST_SEND_MESSAGE) {
        strRequest = createSendMessageRequest(botToken, channelId, content);
    }

    request = strRequest.c_str();

    SSL_write(ssl, request, strlen(request));

    while(true) {
        memset(buffer, 0, sizeof(buffer));
        int n =  SSL_read(ssl, buffer, BUFFER - 1);

        if(n >= 0) {
            buffer[n] = '\0';
        }

        if (n <= 5) {
            break;
        };

        response << buffer;

        if (isResponseChunked(response.str()) != true) {
            break;
        }
    }
    

    return response.str();
}