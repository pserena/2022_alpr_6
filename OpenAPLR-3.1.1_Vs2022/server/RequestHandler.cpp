#include "RequestHandler.h"
#include <string>
using namespace std;




RequestHandler::RequestHandler() {
	vif_.emplace_back(VehicleInfoFinder());
}
RequestHandler::~RequestHandler() {}

void RequestHandler::handle(const string& request) {
	string out;
	vif_.back().getVehicleInformation(request, out);
	cout << out << endl;
}