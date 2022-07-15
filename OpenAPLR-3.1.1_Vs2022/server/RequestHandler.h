#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <atomic>
#include <Windows.h>
#include <map>
#include <mutex>

#include "json.hpp"
#include "VehicleInfoFinder.h"
#include "LoginRequestHandler.h"

using namespace std;
using json = nlohmann::json;

struct Statistics {
	atomic_uint total_queries = 0;
	atomic_uint exact_match = 0;
	atomic_uint partial_match = 0;
	atomic_uint no_match = 0;
	void print() const {
		cout << "Query : " << total_queries << endl;
		cout << "Match : " << exact_match << endl;
		cout << "Partial match : " << partial_match << endl;
		cout << "No match : " << no_match << endl;
	}
};

class RequestHandler {
public:
	RequestHandler();
	virtual ~RequestHandler();
	void handle(UINT_PTR id, string request, function<void(string)> callback);
	void connect(UINT_PTR id);
	void disconnect(UINT_PTR id);

private:
	string GetLoginAccount(UINT_PTR id) {
		if (SessionLoginAccounts.find(id) != SessionLoginAccounts.end())
		{
			return SessionLoginAccounts[id];
		}			

		return "";
	}

	unique_ptr<VehicleInfoFinder> vif_ = make_unique<VehicleInfoFinder>();
	unique_ptr<LoginRequestHandler> loginRequestHandler_ = make_unique<LoginRequestHandler>();

	queue<string> requests_;
	void loginHandler(nlohmann::json requestJson, function<void(string)> callback);
	void plateQueryHandler(UINT_PTR id, nlohmann::json requestJson, function<void(string)> callback);
	atomic_int thread_num_ = 0;
	map<UINT_PTR, string> SessionLoginAccounts;
	map<string, Statistics> statistics_;
};

