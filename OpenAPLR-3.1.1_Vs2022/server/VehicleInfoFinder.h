#pragma once
#include <iostream>

using namespace std;
class VehicleInfoFinder {
public:
	VehicleInfoFinder() = default;
	virtual ~VehicleInfoFinder() = default;
	int getVehicleInformation(const string& plate, string& output);
};