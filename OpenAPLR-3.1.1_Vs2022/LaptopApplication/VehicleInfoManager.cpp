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
	printf("sendVehicleInfo ->%s\n", vehicleData);
	commMan->sendCommunicationData(vehicleData);

	return 0;
}

int VehicleInfoManager::receiveCommunicationData(char* vehicleData) {
	commMan->receiveCommunicationData(vehicleData);

	return 0;
}

int VehicleInfoManager::setRecognizedInfo(string rs, int puid, Mat pimag)
{
	printf("rs:%s puid:%d\n", rs, puid);

	return 0;
}