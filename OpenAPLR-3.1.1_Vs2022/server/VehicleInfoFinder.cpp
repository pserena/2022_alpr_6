#include "VehicleInfoFinder.h"

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <string>
#include <chrono>

#include <Windows.h>
#include <winhttp.h>

#include "json.hpp"


#pragma comment(lib, "winhttp.lib")

using namespace std;
using json = nlohmann::json;

int VehicleInfoFinder::getVehicleInformation(const string& plate, string& output) {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;

    int found = 0;
    json json_output;
   
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
    int priority = 0;
    // below 300 bytes measn no found.
    output.clear();

    do {
        wstring url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:";
        switch (priority) {
        case 0:
            url += wstring(plate.begin(), plate.end());
            break;
        case 1:
            url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:" + wstring(plate.begin(), plate.end()) + L"?";
            break;
        case 2:
            url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:?" + wstring(plate.begin(), plate.end());
            break;
        case 3:
            url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:" + wstring(plate.begin(), plate.end()) + L"~1";
            break;
        case 4:
        default:
            url = L"/solr/sw_alpr_up/select?rows=5&q=plate_number:" + wstring(plate.begin(), plate.end()) + L"~2";
            break;
        }
        output.clear();

        // Create an HTTP request handle.
        if (hConnect)
            hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(),
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
        json_output = json::parse(output);
        found = json_output["response"]["numFound"];
        if ( found > 0)
            break;
    } while (++priority < 5);

    // Report any errors.
    if (!bResults)
        printf("Error %d has occurred.\n", GetLastError());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    auto search_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
    cout << "DB search time : " << search_time << "ms (found :" << found << ") for : " << json_output["responseHeader"]["params"]["q"] << endl;

    return 0;
}
