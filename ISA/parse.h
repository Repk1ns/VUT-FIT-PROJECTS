/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: parse.h
 * Description: Header file.
 */

#ifndef PARSE_H
#define PARSE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <regex>

/**
 * CONSTANTS
 */
#define GET_CLIENT_ID 1
#define GET_GUILDS 2
#define GET_CHANNELS 3
#define GET_MESSAGES 4
#define GET_LAST_MESSAGE_ID 5
#define GET_ACTUAL_MESSAGES 6
#define POST_SEND_MESSAGE 7

#define RATELIMIT_REMAINING "x-ratelimit-remaining:"
#define RATELIMIT_RESET_TIME "x-ratelimit-reset-after:"


/**
 * Function that splits given string by a given delimiter.
 * Similar with explode() function from PHP.
 * It returns vector of strings filled with exploded parts.
 */
std::vector<std::string> explode(const std::string &str, const std::string &delimiter);

/**
 * Function that splits given JSON string to single parts.
 * It returns vector of strings(JSON parts).
 */
std::vector<std::string> explodeJson(const std::string &str);

/**
 * Auxiliary function that removes quotation marks in given string.
 */
std::string removeQuotationMarks(std::string str);

/**
 * Function that gets and returns ID from JSON part.
 */
std::string getID(std::string jsonPart);

/**
 * Function that splits HTTP response and returns HTTP header.
 */
std::string getResponseHeader(const std::string &response);

/**
 * Function that splits HTTP response and returns HTTP body.
 */
std::string getJsonBody(const std::string &response);

/**
 * Auxiliary function that parse message JSON and returns message content.
 */
std::string getMessageContent(std::string message);

/**
 * Function to get authors name or ID (if argument id of function is true) from message JSON.
 */
std::string getMessageAuthor(std::string message, bool id = false);

/**
 * Function that splits response and returns vector of new messages to repeat by bot.
 */
std::vector<std::string> getNewMessages(std::string response);

/**
 * Function that find remaining count of requests which can be send or remaining time to reset limit of requests which can be send.
 * This number is in HTTP header in x-ratelimit-remaining.
 */
int getRatelimit(const std::string &response, const std::string &headerLine);

/**
 * Function to process responses from Discord.
 * Specific processing depends on given function parameter - responseType.
 */
std::string processResponseJson(std::string response, int responseType);

/**
 * Auxiliary function to detect if response is chunked.
 * Required in sendRequest() in part of reading response.
 */
bool isResponseChunked(const std::string &response);

/**
 * Function that verifies given bot token in input argument -t.
 */
bool isWrongToken(const std::string &response);

#endif