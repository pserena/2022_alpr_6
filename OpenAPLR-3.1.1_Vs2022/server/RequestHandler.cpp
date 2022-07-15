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

RequestHandler::RequestHandler() : print_thread(make_unique<thread>(&RequestHandler::printInformation, this)) {
}
RequestHandler::~RequestHandler() {
	quit_ = true;
	print_thread->join();
};

void RequestHandler::plateQueryHandler(UINT_PTR id, nlohmann::json requestJson, function<void(string)> callback) {
	//cout << "Num of thread : " << thread_num_ << endl;
	nlohmann::json responseJson;
	//cout << requestJson.dump() << endl;
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

void RequestHandler::printInformation() {
	Statistics prev_s;
	map<string, uint32_t> query_per_sec;
	while (!quit_) {
		system("cls");
		Statistics s;
		uint32_t total_queyr_per_sec = 0;
		uint32_t q;
		for (const auto& it : statistics_) {
			s.total_queries += it.second.total_queries;
			s.exact_match += it.second.exact_match;
			s.partial_match += it.second.partial_match;
			s.no_match += it.second.no_match;
			cout << "User : " << it.first << endl;
			it.second.print();

			if (query_per_sec.find(it.first) != query_per_sec.end()) {
				q = it.second.total_queries - query_per_sec[it.first];	
			}
			else {
				q = it.second.total_queries;
			}
			cout << "Query per sec : " << q << endl;
			total_queyr_per_sec += q;
			query_per_sec[it.first] = it.second.total_queries;
		}

		cout << "----------------------" << endl;
		cout << "Total" << endl;
		s.print();
		cout << "Query per sec : " << total_queyr_per_sec << endl;
		Sleep(1000);
	}

}

void RequestHandler::connect(UINT_PTR id) {
	cout << "Connect " << id << endl;
}

void RequestHandler::disconnect(UINT_PTR id) {
	cout << "Disconnect " << id << endl;
	SessionLoginAccounts.erase(id);
}
