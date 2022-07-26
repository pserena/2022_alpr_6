#include "LapTopModules.h"
#include <iostream>
#include "json.hpp"

using namespace client;
using json = nlohmann::json;

LoginManager::LoginManager(void)
{
	commMan = NULL;
}
LoginManager::~LoginManager(void)
{
}

int LoginManager::linkCommMag(CommunicationManager* linkCommMan) {
	commMan = linkCommMan;

	return 0;
}

int LoginManager::login(void) {
	inputLoginInfo();
	checkLoginSuccess();

	return 0;
}

int LoginManager::inputLoginInfo(void) {
	cout << "ID : " << endl;
	string id, passwd;
	cin >> id;
	cout << "Password : " << endl;
	cin >> passwd;
	commMan->authenticate(id, passwd);

	return 0;
}

int LoginManager::checkLoginSuccess(void) {
	char ResponseBuffer[8192] = {0, };
	static int retryCount = 0;

	while (true)
	{
		int result = commMan->receiveAuthenticateData(ResponseBuffer);
		if (result >=0 && ResponseBuffer[0] != 0) {
			printf("receiveAuthenticateData JOSN %s\n", ResponseBuffer);
			json responseJson = json::parse(ResponseBuffer);
			if (responseJson["request_type"] == "login")
			{
				if (responseJson["response_code"] == 200)
				{
					printf("checkLogin Success \n");
					return 0;
				}
				else {
					printf("checkLogin Fail \n");
					inputLoginInfo();
					retryCount = 0;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (retryCount++ > 50) {
			commMan->retryNetworkConnectSave(true);
			inputLoginInfo();
			printf("checkLogin Fail timeout \n");
			retryCount = 0;
		}
	}
	return 0;
}

int LoginManager::logout(void) {
	commMan->networkConnectClose();

	return 0;
}
