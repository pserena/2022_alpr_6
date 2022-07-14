#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class LoginRequestHandler final {
public:
	LoginRequestHandler() = default;
	~LoginRequestHandler() = default;
	int login(const nlohmann::json& requestJson, nlohmann::json& responseJson);
};
