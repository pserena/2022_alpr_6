#include "LapTopModules.h"
#include <iostream>

using namespace client;

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
	cout << "ID : " << endl;
	string id, passwd;
	cin >> id;
	cout << "Password : " << endl;
	cin >> passwd;
	commMan->authenticate(id, passwd);

//	int result = comMan.authenticate(strID, strPw);
	return 0;
}

int LoginManager::logout(void) {
	commMan->networkConnectClose();

	return 0;
}
