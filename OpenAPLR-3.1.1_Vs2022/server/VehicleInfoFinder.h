#pragma once
#include <iostream>

using namespace std;
class VehicleInfoFinder final {
public:
	VehicleInfoFinder() = default;
	~VehicleInfoFinder() = default;
	int getVehicleInformation(const string& plate, string& output);

};