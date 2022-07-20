#include "LapTopModules.h"
#include <thread>
#include <map>
#include <iostream>
#include <fstream>
#include "json.hpp"

using namespace client;
using json = nlohmann::json;

map<string, pair<int, ULONGLONG>> mapVehicleNum;
map<int, Mat> mapVehicleImg;
vector<int> matchVehicleNum;

static int receiveThread(VehicleInfoManager* vehicleMan);

VehicleInfoManager::VehicleInfoManager(UIManager* uiManager) {
	ui = uiManager;
	commMan = NULL;
	//tm* tm = localtime(time(NULL));
	log_output_.open("6team.log", std::ofstream::out);
	
}

VehicleInfoManager::~VehicleInfoManager(void) {
	if (log_output_.is_open())
		log_output_.close();
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
	//mapVehicleNum.insert(make_pair(rs, make_pair(puid, time(NULL)));
	mapVehicleNum[rs] = make_pair(puid, GetTickCount64());
	if (!pimag.empty()) {
		Mat copy;
		pimag.copyTo(copy);
		mapVehicleImg.insert(make_pair(puid, copy));
	}
	commMan->sendRecognizedInfo(rs, puid);

	return 0;
}

int VehicleInfoManager::receiveCommunicationData(void)
{
	static time_t errorStartTime = time(NULL);
	static bool receiveError = false;
	char ResponseBuffer[8192] = {0, };
	int result = commMan->receiveCommunicationData(ResponseBuffer);
	if (ResponseBuffer[0] != 0) {
		errorStartTime = time(NULL);
		receiveError = false; 
		//printf("receiveCommunicationData JSON %s\n", ResponseBuffer);
		//string id;
		//cin >> id;
		try {
			//cout << ResponseBuffer << endl;
			json responseJson = json::parse(ResponseBuffer);
			if (responseJson["request_type"] == "query") {
				string plate_number = responseJson["plate_number"];
				int puid = mapVehicleNum.find(plate_number)->second.first;
				if (responseJson["response_code"] == 200 && responseJson["response"]["numFound"] != 0) {
					string strPlateUID = responseJson["plate_uid"];
					int nPlateUID = stoi(strPlateUID);
					auto it = find(matchVehicleNum.begin(), matchVehicleNum.end(), nPlateUID);
					if (it == matchVehicleNum.end()) {
						const string& query = responseJson["responseHeader"]["params"]["q"];
						if (query.find("~") == string::npos) {
							matchVehicleNum.push_back(nPlateUID);
						}
						
						Mat pimag = mapVehicleImg.find(puid)->second;
						json jsonRetPlateInfo = responseJson["response"];

						ui->UpdateVinfo(plate_number, puid, pimag, jsonRetPlateInfo, (int)receiveError);
					}
					//else {
					//	printf("\n\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ %d\n\n\n", nPlateUID);
					//}
				}

				vector<string> vecPlateNum;
				auto request_time = mapVehicleNum.find(plate_number)->second.second;
				int num = responseJson["response"]["docs"].size();
				for (int i = 0; i < num; i++) {
					string plate_number = responseJson["response"]["docs"].at(i)["plate_number"].at(0).get<string>();
					vecPlateNum.push_back(plate_number);
				}
				cout << "REQUEST " << request_time << " " << plate_number << endl;
				cout << "RESPONSE " << GetTickCount64();
				for (auto& s : vecPlateNum) {
					cout << " " << s;
				}
				cout << endl;
				if (log_output_.is_open()) {
					log_output_ << "REQUEST " << request_time << " " << plate_number << endl;
					log_output_ << "RESPONSE " << GetTickCount64();
					for (auto& s : vecPlateNum) {
						log_output_ << " " << s;
					}
					log_output_ << endl;
				}
			}
		}
		catch (json::parse_error& ex)
		{
			printf("\n\n\n###################################### %d\n\n\n", ex.byte);
			//exit(1);
		}
	}
	else {
		if (!receiveError) {
			receiveError = true;
			errorStartTime = time(NULL);
			printf("receive error start = %d\n", errorStartTime);
		}
		else {
			static time_t curTime = time(NULL);
			if (abs(errorStartTime - time(NULL)) > 5) {
				printf("network disconnect error \n");
				errorStartTime = time(NULL);
				Mat pimag;
				json jsonRetPlateInfo;
				ui->UpdateVinfo("", 0, pimag, jsonRetPlateInfo, (int)receiveError);
			}
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
			//std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
		}).detach();
}

static int receiveThread(VehicleInfoManager* vehicleMan)
{
	vehicleMan->receiveCommunicationData();
	//printf("receiveThreadt\n");
	return 0;
}
