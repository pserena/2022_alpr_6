#include "LoginRequestHandler.h"

#include <iostream>
#include <string>
#include <chrono>

#include <Windows.h>

#include <fstream>
#include <sstream> //std::stringstream

#include "AesManager.h"

using namespace std;


bool equals_ignorecase(const string& a, const string& b)
{
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        });
}

int readFileIntoString(const string& path, string& result) {
    struct stat sb {};

    FILE* input_file = fopen(path.c_str(), "r");
    if (input_file == nullptr)
    {
        cerr << "failed to open the file. (\"" << path << "\")" << endl;
        return -1;
    }

    stat(path.c_str(), &sb);
    result.resize(sb.st_size);

    fread(const_cast<char*>(result.data()), sb.st_size, 1, input_file);
    fclose(input_file);

    return 1;
}

vector<string> split(string str, char Delimiter) {
    istringstream iss(str);
    string buffer;

    vector<string> result;

    while (getline(iss, buffer, Delimiter)) {
        result.push_back(buffer);
    }

    return result;
}

int UserAuthotication(string& userId, string& userPassword)
{
    string fileName = "user_accounts.txt";
    string accountInfo;
    int fileStatus = readFileIntoString(fileName, accountInfo);
    if (fileStatus < 0)
    {
        cerr << "failed to get the account info." << endl;
        cerr << "the file(\"" << fileName << "\") " << "must exist in the server run location." << endl;
        return 0;
    }
    cout << "accountInfo: " << accountInfo << endl;

    unsigned char decryptionData[512];
    memset(decryptionData, 0, 512);
    int decryptionDataLength = -1;
    aesDecryption((unsigned char*)accountInfo.c_str(), accountInfo.length(), decryptionData, decryptionDataLength);
    //cout << "accountInfo(decrypted) : " << decryptionData << endl;

    string decryptedAccountInfo = static_cast<std::string>(reinterpret_cast<const char*>(decryptionData));
    vector<string> result = split(decryptedAccountInfo, '\n');
    for (auto cur : result)
    {
        vector<string> idPassword = split(cur, '|');
        string curId = idPassword[0];
        string curPassword = idPassword[1];
        if (equals_ignorecase(userId, curId)
            && equals_ignorecase(userPassword, curPassword))
        {
            return 1;
        }
    }
    return 0;
}

int LoginRequestHandler::login(const nlohmann::json& requestJson, nlohmann::json& responseJson)
{
    auto start_time = std::chrono::milliseconds(GetTickCount64());

    string userId = requestJson["user_id"].get<std::string>();
    //cout << "userId: " << userId << endl;
    string userPassword = requestJson["user_password"].get<std::string>();
    //cout << "userPassword: " << userPassword << endl;

    responseJson["request_type"] = requestJson["request_type"];
    responseJson["user_id"] = requestJson["user_id"];
    responseJson["response_code"] = -100;
    responseJson["response_message"] = "login failed.";

    if (UserAuthotication(userId, userPassword))
    {
        responseJson["response_code"] = 200;
        responseJson["response_message"] = "login succeed.";

        //cout << "responseJson(changed): " << responseJson << endl;
    }

    auto process_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
    cout << "Login process time : " << process_time << "ms." << endl;
    return 0;
}
