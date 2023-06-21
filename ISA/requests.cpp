/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: requests.cpp
 * Description: Contains functions that assembly and returns HTTP requests.
 */

#include "requests.h"

using namespace std;


string createIDRequest(string botToken)
{
    string method = "GET ";
    string alias = "/api/v6/users/@me ";
    string http = "HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string endOfHeader = "\r\n\r\n";

    string request = method + alias + http + host + authorization + botToken + endOfHeader;

    return request;
}


string createGuildsRequest(string botToken)
{
    string method = "GET ";
    string alias = "/api/v6/users/@me/guilds ";
    string http = "HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string endOfHeader = "\r\n\r\n";

    string request = method + alias + http + host + authorization + botToken + endOfHeader;

    return request;
}


string createChannelRequest(string botToken, string guildId)
{
    string method = "GET ";
    string alias = "/api/v6/guilds/" + guildId + "/channels ";
    string http = "HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string endOfHeader = "\r\n\r\n";

    string request = method + alias + http + host + authorization + botToken + endOfHeader;

    return request;  
}


string createMessagesRequest(string botToken, string channelId)
{
    string method = "GET ";
    string alias = "/api/v6/channels/" + channelId + "/messages ";
    string http = "HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string endOfHeader = "\r\n\r\n";

    string request = method + alias + http + host + authorization + botToken + endOfHeader;

    return request;
}


string createActualMessagesRequest(string botToken, string channelId, string lastMessageId)
{
    string method = "GET ";
    string alias = "/api/v6/channels/" + channelId + "/messages?after=" + lastMessageId;
    string http = " HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string endOfHeader = "\r\n\r\n";

    string request = method + alias + http + host + authorization + botToken + endOfHeader;

    return request;
}


string createSendMessageRequest(string botToken, string channelId, string content)
{
    string body = "{\"content\": \"" + content + "\", \"tts\": \"false\"}";
    int bodySize = body.size();

    string method = "POST ";
    string alias = "/api/v6/channels/" + channelId + "/messages ";
    string http = "HTTP/1.1\r\n";
    string host = "Host: discord.com\r\n";
    string authorization = "Authorization: Bot ";
    string contentType = "Content-Type: application/json\r\n";
    string contentLength = "Content-Length: " + to_string(bodySize);
    string endOfHeader = "\r\n\r\n";
    

    string request = method + alias + http + host + authorization + botToken + "\r\n" + contentType + contentLength + endOfHeader + body;

    return request;
}
