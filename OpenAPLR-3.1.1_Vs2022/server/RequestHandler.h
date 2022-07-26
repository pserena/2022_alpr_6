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
	Statistics& operator=(const Statistics& o) {
		total_queries = o.total_queries.load();
		exact_match = o.exact_match.load();
		partial_match = o.partial_match.load();
		no_match = o.no_match.load();
		return *this;
	}
};

class RequestHandler {
public:
	RequestHandler();
	virtual ~RequestHandler();
	void handle(UINT_PTR id, string request, function<void(string)> callback);
	void connect(UINT_PTR id);
	void disconnect(UINT_PTR id);
	void fileWriteInformation();

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
	bool quit_ = false;
	unique_ptr<thread> print_thread;
	void printInformation();
};

