#include "LapTopModules.h"

using namespace client;

VehicleInfoManager::VehicleInfoManager(void) {
}

VehicleInfoManager::~VehicleInfoManager(void) {
}

int VehicleInfoManager::linkCommMag(CommunicationManager* linkCommMan) {
	commMan = linkCommMan;

	return 0;
}

int VehicleInfoManager::sendVehicleInfo(unsigned char* vehicleData) {
	commMan->sendCommunicationData(vehicleData);

	return 0;
}

int VehicleInfoManager::receiveCommunicationData(unsigned char* vehicleData) {
	commMan->sendCommunicationData(vehicleData);

	return 0;
}