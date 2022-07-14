#include "LoginRequestHandler.h"

#include <iostream>
#include <string>
#include <chrono>

#include <Windows.h>
#include <winhttp.h>


#pragma comment(lib, "winhttp.lib")

using namespace std;

int LoginRequestHandler::login(const nlohmann::json& requestJson, nlohmann::json& responseJson) {
    auto start_time = std::chrono::milliseconds(GetTickCount64());

    string userId = requestJson["user_id"].get<std::string>();
    string userPassword = requestJson["user_password"].get<std::string>();
    
    responseJson["request_type"] = requestJson["request_type"];
    responseJson["user_id"] = requestJson["user_id"];

    if (userId == "daniel" && userPassword == "1qaz2wsx")
    {
        responseJson["response_code"] = 200;
        responseJson["response_message"] = "login succeed.";
    }
    else
    {
        responseJson["response_code"] = -100;
        responseJson["response_message"] = "login failed.";
    }

    auto process_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
    cout << "Login process time : " << process_time << "ms." << endl;
    return 0;
}
