#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class VehicleInfoFinder final {
public:
	VehicleInfoFinder();
	~VehicleInfoFinder() = default;
	int getVehicleInformation(const nlohmann::json& requestJson, nlohmann::json& responseJson);

private:
	vector<string> rules;
};
