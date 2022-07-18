#include "LapTopModules.h"
#include <thread>
#include <map>

using namespace client;

map<string, int> mapVehicleNum;
map<int, Mat> mapVehicleImg;

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
	char ResponseBuffer[8192] = {0, };
	int result = commMan->receiveCommunicationData(ResponseBuffer);
	//printf("receiveCommunicationData %s\n", ResponseBuffer);
	if (ResponseBuffer[0] != 0) {
		printf("receiveCommunicationData JOSN %s\n", ResponseBuffer);
	}
	return 0;
}

void VehicleInfoManager::recevieThreadStart(void) {
	static bool startThread = false;
	if (!startThread) {
		timer_start(receiveThread, 20);
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
	vehicleMan->receiveCommunicationData();
	//printf("receiveThreadt\n");
	return 0;
}
