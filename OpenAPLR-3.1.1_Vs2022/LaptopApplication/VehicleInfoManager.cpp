#include "LapTopModules.h"
#include <thread>
#include <map>
#include "json.hpp"

using namespace client;
using json = nlohmann::json;

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

/*
int VehicleInfoManager::sendVehicleInfo(unsigned char* vehicleData) {
	printf("sendVehicleInfo ->%s\n", vehicleData);
	commMan->sendCommunicationData(vehicleData);

	return 0;
}*/

int VehicleInfoManager::setRecognizedInfo(string rs, int puid, Mat pimag)
{
	mapVehicleNum.insert(make_pair(rs, puid));
	if (mapVehicleImg.find(puid) == mapVehicleImg.end()) {
		mapVehicleImg.insert(make_pair(puid, pimag));
	}
	commMan->sendRecognizedInfo(rs, puid);

	return 0;
}

int VehicleInfoManager::receiveCommunicationData(void)
{
	char ResponseBuffer[8192] = {0, };
	int result = commMan->receiveCommunicationData(ResponseBuffer);
	if (ResponseBuffer[0] != 0) {
		printf("receiveCommunicationData JOSN %s\n", ResponseBuffer);
		try {
			json responseJson = json::parse(ResponseBuffer);
			if (responseJson["request_type"] == "query") {
				string plate_number = responseJson["plate_number"];
				int puid = mapVehicleNum.find(plate_number)->second;
				Mat pimag = mapVehicleImg.find(puid)->second;
				json jsonRetPlateInfo = responseJson["response"];

				ui->UpdateVinfo(plate_number, puid, pimag, jsonRetPlateInfo);
			}
		}
		catch (json::parse_error& ex)
		{
			printf("\n\n\n###################################### %d\n\n\n", ex.byte);
		}
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
