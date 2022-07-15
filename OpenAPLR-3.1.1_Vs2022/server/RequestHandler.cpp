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

void RequestHandler::plateQueryHandler(UINT_PTR id, nlohmann::json requestJson, function<void(string)> callback) {
	//cout << "Num of thread : " << thread_num_ << endl;
	nlohmann::json responseJson;
	cout << requestJson.dump() << endl;
	vif_->getVehicleInformation(requestJson, responseJson);
	if (responseJson["response"]["numFound"] == 0) {
		++statistics_[SessionLoginAccounts[id]].no_match;
	}
	else {
		const string& query = responseJson["responseHeader"]["params"]["q"];
		if (query.find("~") == string::npos)
			++statistics_[SessionLoginAccounts[id]].exact_match;
		else
			++statistics_[SessionLoginAccounts[id]].partial_match;		
	}
	/* This is test code */
	cout << "User : " << SessionLoginAccounts[id] << endl;
	statistics_[SessionLoginAccounts[id]].print();
	
	callback(responseJson.dump());
	--thread_num_;
}

void RequestHandler::loginHandler(nlohmann::json requestJson, function<void(string)> callback) {

	//cout << "Num of thread : " << thread_num_ << endl;
	nlohmann::json responseJson;
	loginRequestHandler_->login(requestJson, responseJson);

	if (responseJson["response_code"] == 200)
	{
		SessionLoginAccounts[requestJson["id"]] = requestJson["user_id"];
	}

#if 0
	for (auto iter = SessionLoginAccounts.begin(); iter != SessionLoginAccounts.end(); iter++)
	{
		cout << "Key : " << iter->first << ", Value : " << iter->second << endl;
	}
#endif
	
	callback(move(responseJson.dump()));
	//cout << "[loginHandler] " << responseJson.dump() << endl;
	
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
#if 0
		map<UINT_PTR, string>::iterator iter;
		for (iter = SessionLoginAccounts.begin(); iter != SessionLoginAccounts.end(); iter++)
		{
			cout << "Key : " << iter->first << ", Value : " << iter->second << endl;
		}
		cout << "id: " << id << endl;
#endif
		const auto& user_id = SessionLoginAccounts.find(id);
		if (user_id == SessionLoginAccounts.end())
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
		statistics_[user_id->second].total_queries++;

		thread t(&RequestHandler::plateQueryHandler, this, id, move(RequestJson), move(callback));
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
