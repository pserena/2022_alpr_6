#include "VehicleInfoFinder.h"

#include <iostream>
#include <string>
#include <chrono>

#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

using namespace std;

int VehicleInfoFinder::getVehicleInformation(const string& plate, string& output) {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
   
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
    int fuzzy_search = 0;
    // below 300 bytes measn no found.
    output.clear();

    while (output.length() < 600 && fuzzy_search < 3) {
        output.clear();
        wstring url = L"/solr/swarchitect_alpr/select?rows=5&q=plate_number:" + wstring(plate.begin(), plate.end());
        if (fuzzy_search != 0) {
            url += L"~" + to_wstring(fuzzy_search);
        }
        fuzzy_search++;

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
    }

    // Report any errors.
    if (!bResults)
        printf("Error %d has occurred.\n", GetLastError());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    auto search_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
    cout << "DB search time : " << search_time << "ms - fuzzy : " << fuzzy_search - 1 << endl;

    return 0;
}
