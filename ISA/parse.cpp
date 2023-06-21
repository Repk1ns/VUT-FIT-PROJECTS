/**
 * ISA 2020/2021 - Discord Bot
 * Author: Vojtěch Mimochodek
 * Login: xmimoc01
 * File: parse.cpp
 * Description: Contains function to parse HTTP responses and auxiliary function.
 */

#include "parse.h"
#include "errors.h"

using namespace std;


vector<string> explode(const string &str, const string &delimiter)
{
    vector<string> result;
 
    int delimiterLength = delimiter.length();

    if (delimiterLength == 0) {
        return result;
    }

    int i = 0;
    int k = 0;
    int stringLength = str.length();

    while(i < stringLength) {

        int j = 0;

        while(i+j < stringLength && j < delimiterLength && str[i+j] == delimiter[j]) {
            j++;
        }
        if (j == delimiterLength) {
            result.push_back(str.substr(k, i - k));
            i += delimiterLength;
            k = i;
        }
        else {
            i++;
        }
    }

    result.push_back(str.substr(k, i - k));

    return result;
}


vector<string> explodeJson(const string &str)
{
    vector<string> result;

    int i = 0;
    int k = 0;
    int stringLength = str.length();
    int brackets = 0;

    while (i < stringLength) {
        if (str[i] == '{') {
            brackets++;
        }
        if (str[i] == '}') {
            if (brackets == 1) {
                result.push_back(str.substr(k + 3, i - k));
                k = i;
                brackets--;
            }
            else {
                brackets--;
            }
        }
        i++;
    }

    return result;
}


string removeQuotationMarks(string str)
{
    str.erase(str.begin());
    str.erase(str.end()-1);

    return str;
}


string getID(string jsonPart)
{
    vector<string> explodedJsonBody = explode(jsonPart, ",");
    vector<string> IDPart = explode(explodedJsonBody[0], " ");
    string ID = IDPart[1];

    ID = removeQuotationMarks(ID);

    return ID;
}


string getResponseHeader(const string &response)
{
    string headerEnd = "\r\n\r\n";
    int headerEndIndex = response.find(headerEnd);
    string header = response.substr(0, headerEndIndex);

    return header;
}


string getJsonBody(const string &response)
{
    string headerEnd = "\r\n\r\n";
    int bodyBegin = response.find(headerEnd);
    string jsonBody = response.substr(bodyBegin + headerEnd.length());

    return jsonBody;
}


string getMessageContent(string message)
{
    string startOfContent = "\"content\": ";
    string endOfContent = ", \"channel_id\"";

    unsigned start = message.find(startOfContent) + startOfContent.length();
    unsigned end = message.find(endOfContent);

    string content = message.substr(start, end-start);
    content = removeQuotationMarks(content);

    return content;
}

string getMessageAuthor(string message, bool id)
{
    string startOfAuthorJson = "\"author\": ";
    string endOfAuthorJson = "},";

    unsigned start = message.find(startOfAuthorJson);
    unsigned end = message.find(endOfAuthorJson);

    string author = message.substr(start, end-start);

    vector<string> explodedAuthorJson = explode(author, ",");

    if (id == true) {
        vector<string> explodedAuthorId = explode(explodedAuthorJson[0], " ");
        string authorId = explodedAuthorId[2];
        authorId = removeQuotationMarks(authorId);

        return authorId;
    }
    else {
        vector<string> explodedAuthorName = explode(explodedAuthorJson[1], " ");
        string authorName = explodedAuthorName[2];
        authorName = removeQuotationMarks(authorName);

        return authorName;
    }
}


vector<string> getNewMessages(string response)
{
    string jsonBody = getJsonBody(response);

    vector<string> messages = explodeJson(jsonBody);
    int messagesCount = messages.size();

    if (messagesCount == 0) {
        messages.clear();

        return messages;
    }

    return messages;

}


int getRatelimit(const string &response, const string &headerLine)
{
    int result;
    string header = getResponseHeader(response);
    vector<string> explodedHeader = explode(header, "\r\n");
    int elements = explodedHeader.size();
    for(int i = 0; i < elements; i++) {
        if(explodedHeader[i].find(headerLine) != string::npos) {
            vector<string> remainingRatelimit = explode(explodedHeader[i], " ");
            result = stoi(remainingRatelimit[1]);
            break;
        }
    }
    
    return result;
}


string processResponseJson(string response, int responseType)
{
    string jsonBody = getJsonBody(response);

    if (responseType == GET_CLIENT_ID || responseType == GET_GUILDS) {

        if (jsonBody == "[]")
        {
            errorCall("BOT isn't member of any guilds - servers!", DISCORD_ERROR);
        }

        if (isWrongToken(response)) {
            errorCall("Wrong BOT Token!", DISCORD_ERROR);
        }

        string clientOrGuildID = getID(jsonBody);

        return clientOrGuildID;

    }
    else if (responseType == GET_CHANNELS) {

        if (jsonBody == "[]")
        {
            errorCall("Guild - Server doesn't have any channels!", DISCORD_ERROR);
        }

        vector<string> channels = explodeJson(jsonBody);
        string isaBotChannel = "";

        for (long unsigned int i = 0; i < channels.size(); i++) {
            if (channels[i].find("isa-bot") != string::npos) {
                isaBotChannel = channels[i];
                break;
            }
        }

        if (isaBotChannel != "") {
            string isaBotChannelID = getID(isaBotChannel);

            return isaBotChannelID;
        }
        else {
            errorCall("Server doesn't have isa-bot channel!", DISCORD_ERROR);
        }
        

        return "";
    }
    else if (responseType == GET_LAST_MESSAGE_ID) {
        vector<string> messages = explodeJson(jsonBody);

        if (messages.size() == 0) {
            return "";
        }

        string lastMessageID = getID(messages[0]);

        return lastMessageID;
    }

    return "";

}


bool isResponseChunked(const string &response)
{
    string header = getResponseHeader(response);
    if (header.find("Transfer-Encoding: chunked") == string::npos) {
        return false;
    }

    return true;
}

bool isWrongToken(const string &response)
{
    string header = getResponseHeader(response);
    if (header.find("401 Unauthorized") == string::npos) {
        return false;
    }

    return true;
}