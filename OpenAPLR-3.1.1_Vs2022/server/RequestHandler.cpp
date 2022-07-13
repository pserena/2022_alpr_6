#include "RequestHandler.h"
#include <string>
#include <algorithm>
#include <random>
#include <thread>
using namespace std;

RequestHandler::RequestHandler() = default;
RequestHandler::~RequestHandler() = default;

void RequestHandler::plateQueryHandler(string plate, function<void(string)> callback) {
	cout << "Num of thread : " << thread_num_ << endl;
	string output;
	vif_->getVehicleInformation(plate, output);
	callback(move(output));
	--thread_num_;
}

void RequestHandler::handle(UINT_PTR id, string request, function<void(string)> callback) {
	++thread_num_;
#if 0 // After implemented log-in
	if (!loginStatus(id))
		callback("Need to log-in");
#endif;
	thread t(&RequestHandler::plateQueryHandler, this, move(request), move(callback));
	t.detach();
}

void RequestHandler::connect(UINT_PTR id) {
	cout << "Connect " << id << endl;
	login[id] = false;
}

void RequestHandler::disconnect(UINT_PTR id) {
	cout << "Disconnect " << id << endl;
	login.erase(id);
}