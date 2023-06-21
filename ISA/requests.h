/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: requests.h
 * Description: Header file.
 */

#ifndef REQUESTS_H
#define REQUESTS_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>


/**
 * Returns assembled GET request to get client ID of bot.
 */
std::string createIDRequest(std::string botToken);

/**
 * Returns assembled GET request to get guilds/servers where bot is a member.
 */
std::string createGuildsRequest(std::string botToken);

/**
 * Returns assembled GET request to get channels of guild/server.
 */
std::string createChannelRequest(std::string botToken, std::string guildId);

/**
 * Returns assembled GET request to get messages in channel.
 */
std::string createMessagesRequest(std::string botToken, std::string channelId);

/**
 * Returns assembled GET request to get new unprocessed messages in channel.
 */
std::string createActualMessagesRequest(std::string botToken, std::string channelId, std::string lastMessageId);

/**
 * Returns assembled POST request to send a repeated message.
 */
std::string createSendMessageRequest(std::string botToken, std::string channelId, std::string content);

#endif