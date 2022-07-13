#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <atomic>
#include <Windows.h>
#include <map>
#include "VehicleInfoFinder.h"

using namespace std;
class RequestHandler {
public:
	RequestHandler();
	virtual ~RequestHandler();
	void handle(UINT_PTR id, string request, function<void(string)> callback);
	void connect(UINT_PTR id);
	void disconnect(UINT_PTR id);

private:
	bool loginStatus(UINT_PTR id) {
		if (login.find(id) != login.end())
			return login[id];
		return false;
	}

	unique_ptr<VehicleInfoFinder> vif_ = make_unique<VehicleInfoFinder>();
	queue<string> requests_;
	void plateQueryHandler(string plate, function<void(string)> callback);
	atomic_int thread_num_ = 0;
	map<UINT_PTR, bool> login;
};