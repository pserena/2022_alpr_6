#include "VehicleInfoFinder.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include <Windows.h>
#include <winhttp.h>

using json = nlohmann::json;

#pragma comment(lib, "winhttp.lib")

using namespace std;

VehicleInfoFinder::VehicleInfoFinder() {
    char buf[4096] = { 0 };
    ifstream is("server_config.json");
    if (is.is_open()) {
        is.read(buf, sizeof(buf));
        is.close();
        try {
            json s = json::parse(buf);
            rules = move(s["search"].get<vector<string>>());
#if 0
            for (auto i : rules)
                cout << i << endl;
#endif
        }
        catch (json::parse_error& ex) {
            cout << "Failed to parse server_config.json" << endl;
            return;
        }
    }
}

int VehicleInfoFinder::getVehicleInformation(const nlohmann::json& requestJson, nlohmann::json& responseJson) {

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = 0;
    BOOL  bResults = FALSE;

    string output;
    int found = 0;
   
    HINTERNET  hSession = NULL;
    HINTERNET  hConnect = NULL;
    HINTERNET  hRequest = NULL;
    auto start_time = std::chrono::milliseconds(GetTickCount64());

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"127.0.0.1",
            8983, 0);

    string plateNumber = requestJson["plate_number"].get<std::string>();
    wstring wplate = wstring(plateNumber.begin(), plateNumber.end());

    vector<wstring> priority = {
        wplate
    };

    for (auto rule : rules) {
        size_t start_pos = rule.find("PL");
        if (start_pos == string::npos)
            continue;
        rule.replace(start_pos, 2, plateNumber);
        priority.emplace_back(wstring(rule.begin(), rule.end()));
    }
    wstring url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:";
    int cnt = 0;

    try {

        do {
            wstring query_string = url + priority[cnt];
   
            output.clear();

            // Create an HTTP request handle.
            if (hConnect)
                hRequest = WinHttpOpenRequest(hConnect, L"GET", query_string.c_str(),
                    NULL, WINHTTP_NO_REFERER,
                    WINHTTP_DEFAULT_ACCEPT_TYPES,
                    0);

            // Send a request.
            if (hRequest)
                bResults = WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0,
                    0, 0);

            // End the request.
            if (bResults)
                bResults = WinHttpReceiveResponse(hRequest, NULL);

            // Keep checking for data until there is nothing left.
            if (bResults)
            {
                do
                {
                    // Check for available data.
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                        printf("Error %u in WinHttpQueryDataAvailable.\n",
                            GetLastError());

                    // Allocate space for the buffer.
                    pszOutBuffer = new char[dwSize + 1];
                    if (!pszOutBuffer)
                    {
                        printf("Out of memory\n");
                        dwSize = 0;
                    }
                    else
                    {
                        // Read the data.
                        ZeroMemory(pszOutBuffer, dwSize + 1);

                        if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                            dwSize, &dwDownloaded))
                            printf("Error %u in WinHttpReadData.\n", GetLastError());

                        // TODO : This is memcpy. If there are performance issue, need to way to avoid memcpy.
                        output += pszOutBuffer;
                        // Free the memory allocated to the buffer.
                        delete[] pszOutBuffer;
                    }
                } while (dwSize > 0);
            }

            //cout << "[getVehicleInformation] " << output << endl;

            responseJson = json::parse(output.c_str());
            found = responseJson["response"]["numFound"];

            if ( found > 0)
                break;

        } while (++cnt < priority.size());
    }
    catch (...)
    {
        cerr << "the db is not working. check the status of the db." << endl;
        return -1;    
    }

    // Report any errors.
    if (!bResults)
        printf("Error %d has occurred.\n", GetLastError());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    auto search_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
    //cout << "DB search time : " << search_time << "ms (found :" << found << ") for : " << responseJson["responseHeader"]["params"]["q"] << endl;

    responseJson["request_type"] = requestJson["request_type"];
    responseJson["plate_number"] = requestJson["plate_number"];
    responseJson["plate_uid"] = requestJson["plate_uid"];
    responseJson["response_code"] = 200;
    responseJson["response_message"] = "query succeed.";
    //cout << requestJson << endl << endl << responseJson << endl;

    return 0;
}
