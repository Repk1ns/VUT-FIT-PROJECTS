/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: main.cpp
 * Description: Main file of program.
 */

#include "errors.h"
#include "input.h"
#include "requests.h"
#include "parse.h"
#include "connection.h"

using namespace std;


int main(int argc, char* argv[])
{
    string botToken = parseArguments(argc, argv);

    connection conn = initConnection();

    string lastMessageID = "";
    bool emptyChannel = false;

    string clientIdResponse = sendRequest(conn.ssl, GET_CLIENT_ID, botToken);
    string clientId = processResponseJson(clientIdResponse, GET_CLIENT_ID);

    string guildResponse = sendRequest(conn.ssl, GET_GUILDS, botToken);
    string guildId = processResponseJson(guildResponse, GET_GUILDS);

    string channelsResponse = sendRequest(conn.ssl, GET_CHANNELS, botToken, guildId);
    string channelId = processResponseJson(channelsResponse, GET_CHANNELS);

    while(true) {
        string firstMessagesResponse = sendRequest(conn.ssl, GET_MESSAGES, botToken, "", channelId);
        lastMessageID = processResponseJson(firstMessagesResponse, GET_LAST_MESSAGE_ID);

        if (lastMessageID == "") {
            emptyChannel = true;
        }
        else {
            if (emptyChannel == true) {
                string firstMessage = getJsonBody(firstMessagesResponse);
                string rawContent = getMessageContent(firstMessage);
                string author = getMessageAuthor(firstMessage);
                string content = "echo: " + author + " - " + rawContent;
                string sendedMessage = sendRequest(conn.ssl, POST_SEND_MESSAGE, botToken, "", channelId, "", content);
                if (isVerboseActivated == true) {
                    cout << content << endl;
                }
            }
            break;
        }

        sleep(3);
    }
    

    while(true) {
        string messagesResponse = sendRequest(conn.ssl, GET_ACTUAL_MESSAGES, botToken, "", channelId, lastMessageID);
        vector<string> newMessages = getNewMessages(messagesResponse);
        
        int messagesCount = newMessages.size();
        if (messagesCount != 0) {

            int ratelimit;
            int messagesResetTime;

            for (int i = messagesCount - 1; i >= 0; i--)
            {
                string authorId = getMessageAuthor(newMessages[i], true);
                string author = getMessageAuthor(newMessages[i]);

                if (authorId != clientId && author.find("bot") == string::npos) {
                    string rawContent = getMessageContent(newMessages[i]);
                    string content = "echo: " + author + " - " + rawContent;
                    string sendedMessage = sendRequest(conn.ssl, POST_SEND_MESSAGE, botToken, "", channelId, "", content);

                    if (isVerboseActivated == true) {
                        string verboseContent = "isa-bot - " + author + ": " + rawContent;
                        cout << verboseContent << endl;
                    }

                    lastMessageID = getID(newMessages[i]);

                    ratelimit = getRatelimit(sendedMessage, RATELIMIT_REMAINING);
                    messagesResetTime = getRatelimit(sendedMessage, RATELIMIT_RESET_TIME);

                    if(ratelimit == 0) {
                        sleep(messagesResetTime + 1);
                    }
                }
            }

            sleep(messagesResetTime);
            newMessages.clear();
        }
        else {
            int resetTime = getRatelimit(messagesResponse, RATELIMIT_RESET_TIME);
            sleep(resetTime);
        }
    }

    clearConnection(conn);

    return 0;
}