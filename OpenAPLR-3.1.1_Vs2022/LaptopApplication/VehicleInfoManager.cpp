#include "LapTopModules.h"
#include <thread>

using namespace client;

static int receiveThread(VehicleInfoManager* vehicleMan);

VehicleInfoManager::VehicleInfoManager(UIManager* uiManager) {
	ui = uiManager;
	commMan = NULL;
}

VehicleInfoManager::~VehicleInfoManager(void) {
}

int VehicleInfoManager::linkCommMag(CommunicationManager* linkCommMan) {
	commMan = linkCommMan;

	return 0;
}

int VehicleInfoManager::VehicleInfoReceiveStart(void) {
	recevieThreadStart();
	return 0;
}

int VehicleInfoManager::sendVehicleInfo(unsigned char* vehicleData) {
	printf("sendVehicleInfo ->%s\n", vehicleData);
	commMan->sendCommunicationData(vehicleData);

	return 0;
}

int VehicleInfoManager::setRecognizedInfo(string rs, int puid, Mat pimag)
{
	commMan->sendRecognizedInfo(rs, puid);

	return 0;
}

int VehicleInfoManager::receiveCommunicationData(void)
{
	char ResponseBuffer[8192];
	int result = commMan->receiveCommunicationData(ResponseBuffer);
	//if (result < 0)
	//    commMan->retryNetworkConnect();
	return 0;
}

void VehicleInfoManager::recevieThreadStart(void) {
	static bool startThread = false;
	if (!startThread) {
		timer_start(receiveThread, 100);
		startThread = true;
	}
}

void VehicleInfoManager::timer_start(std::function<void(VehicleInfoManager*)> func, unsigned int interval)
{
	std::thread([func, this, interval]() {
		while (true)
		{
			func(this);
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
		}).detach();
}

static int receiveThread(VehicleInfoManager* vehicleMan)
{
	char ResponseBuffer[8192];
	vehicleMan->receiveCommunicationData();
	//printf("receiveThreadt\n");
	return 0;
}
