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

void RequestHandler::handle(string request, function<void(string)> callback) {
	++thread_num_;
	thread t(&RequestHandler::plateQueryHandler, this, move(request), move(callback));
	t.detach();
}