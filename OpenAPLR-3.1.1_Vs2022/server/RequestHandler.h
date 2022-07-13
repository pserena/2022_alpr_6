#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <atomic>

#include "VehicleInfoFinder.h"

using namespace std;
class RequestHandler {
public:
	RequestHandler();
	virtual ~RequestHandler();
	void handle(string request, function<void(string)> callback);

private:
	unique_ptr<VehicleInfoFinder> vif_ = make_unique<VehicleInfoFinder>();
	queue<string> requests_;
	void plateQueryHandler(string plate, function<void(string)> callback);
	atomic_int thread_num_ = 0;
};