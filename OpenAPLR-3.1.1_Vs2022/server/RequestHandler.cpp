#include "RequestHandler.h"
#include <string>
#include <algorithm>
#include <random>
#include <thread>

using namespace std;

bool iequals(const string& a, const string& b)
{
	return std::equal(a.begin(), a.end(),
		b.begin(), b.end(),
		[](char a, char b) {
			return tolower(a) == tolower(b);
		});
}

RequestHandler::RequestHandler() = default;
RequestHandler::~RequestHandler() = default;

void RequestHandler::plateQueryHandler(nlohmann::json requestJson, function<void(string)> callback) {
	cout << "Num of thread : " << thread_num_ << endl;
	nlohmann::json responseJson;
	vif_->getVehicleInformation(requestJson, responseJson);
	callback(move(responseJson.dump()));
	--thread_num_;
}

void RequestHandler::loginHandler(nlohmann::json requestJson, function<void(string)> callback) {

	cout << "Num of thread : " << thread_num_ << endl;
	nlohmann::json responseJson;
	loginRequestHandler_->login(requestJson, responseJson);

	if (responseJson["response_code"] == 200)
	{
		SessionLoginAccounts[requestJson["id"]] = requestJson["user_id"];
	}

	map<UINT_PTR, string>::iterator iter;
	for (iter = SessionLoginAccounts.begin(); iter != SessionLoginAccounts.end(); iter++)
	{
		cout << "Key : " << iter->first << ", Value : " << iter->second << endl;
	}
	
	callback(move(responseJson.dump()));
	cout << "[loginHandler] " << responseJson.dump() << endl;
	
	--thread_num_;
}

void RequestHandler::handle(UINT_PTR id, string requestString, function<void(string)> callback) {
	++thread_num_;

	auto RequestJson = json::parse(requestString);
	string RequestType = RequestJson["request_type"].get<std::string>();
	if (iequals(RequestType, "login"))
	{
		RequestJson["id"] = id;
		thread t(&RequestHandler::loginHandler, this, move(RequestJson), move(callback));
		t.detach();
	}
	else
	{
		map<UINT_PTR, string>::iterator iter;
		for (iter = SessionLoginAccounts.begin(); iter != SessionLoginAccounts.end(); iter++)
		{
			cout << "Key : " << iter->first << ", Value : " << iter->second << endl;
		}
		cout << "id: " << id << endl;

		if (SessionLoginAccounts.find(id) == SessionLoginAccounts.end())
		{
			auto ResponseJson = R"(
				 {
				  "request_type": "query",
				  "response_code": -100,
				  "response_message": "need to login."
				 }
			)"_json;
			ResponseJson["plate_number"] = RequestJson["plate_number"];
			ResponseJson["plate_uid"] = RequestJson["plate_uid"];

			callback(ResponseJson.dump());
			return;
		}

		thread t(&RequestHandler::plateQueryHandler, this, move(RequestJson), move(callback));
		t.detach();
	}
}

void RequestHandler::connect(UINT_PTR id) {
	cout << "Connect " << id << endl;
}

void RequestHandler::disconnect(UINT_PTR id) {
	cout << "Disconnect " << id << endl;
	SessionLoginAccounts.erase(id);
}
