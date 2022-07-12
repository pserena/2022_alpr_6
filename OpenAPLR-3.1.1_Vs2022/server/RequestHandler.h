#pragma once
#include <iostream>
#include <vector>
#include "VehicleInfoFinder.h"

using namespace std;
class RequestHandler {
public:
	RequestHandler();
	virtual ~RequestHandler();

	void handle(const string& request);

private:
	vector<VehicleInfoFinder> vif_;
};