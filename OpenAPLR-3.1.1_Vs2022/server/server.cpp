// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string.h>
#include "NetworkTCP.h"
#include <Windows.h>
#include <db.h> 
#include <unordered_set>
#include <memory>
#include <chrono>

#include <stdio.h>
#include <thread>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")


using namespace std;

/* This is sample code for HTTP to communicate with Solr DB */
void httpTest() {
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
    HINTERNET  hSession = NULL,
        hConnect = NULL,
        hRequest = NULL;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect(hSession, L"www.microsoft.com",
            INTERNET_DEFAULT_HTTPS_PORT, 0);

    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL,
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

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
                else
                    printf("%s", pszOutBuffer);

                // Free the memory allocated to the buffer.
                delete[] pszOutBuffer;
            }
        } while (dwSize > 0);
    }


    // Report any errors.
    if (!bResults)
        printf("Error %d has occurred.\n", GetLastError());

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}

bool doPartitionSearch(DB* dbp, const string& plate, char* out, u_int32_t out_len) {
    if (plate.size() > 7)
        return false;
    DBT key;
    DBT data;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    data.data = out;
    data.ulen = out_len;
    data.flags = DB_DBT_USERMEM;
    key.data = (void *)plate.c_str(); 
    key.size = static_cast<u_int32_t>(plate.length()) + 1U;
    if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND)
            return true;
    for (char c = '0'; c <= '9'; c++) {
        if (doPartitionSearch(dbp, plate + c, out, out_len))
            return true;
    }
    for (char c = 'A'; c <= 'Z'; c++) {
        if (doPartitionSearch(dbp, plate + c, out, out_len))
            return true;
    }
     return false;
}

bool partialMatch(DB* dbp, char* plate, char* out, u_int32_t out_len) {
    /* Zero out the DBTs before using them. */
    return doPartitionSearch(dbp, string(plate), out, out_len);
}

int main()
{
    TTcpListenPort* TcpListenPort;
    TTcpConnectedPort* TcpConnectedPort;
    unordered_set<shared_ptr<TTcpConnectedPort>> connected_ports;
    struct sockaddr_in cli_addr;
    socklen_t          clilen;
    bool NeedStringLength = true;
    unsigned short PlateStringLength;
    char PlateString[1024];
    char DBRecord[2048];
    DB* dbp; /* DB structure handle */
    u_int32_t flags; /* database open flags */
    int ret; /* function return value */
    ssize_t result;
    /* TODO : Delete */
    //httpTest();
    /* Initialize the structure. This
     * database is not opened in an environment,
     * so the environment pointer is NULL. */
    ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB Create Error\n");
        return -1;
    }
    /* Database open flags */
    flags = DB_CREATE; /* If the database does not exist,
     * create it.*/
     /* open the database */
    ret = dbp->open(dbp, /* DB structure pointer */
        NULL, /* Transaction pointer */
        "licenseplate.db", /* On-disk file that holds the database. */
        NULL, /* Optional logical database name */
        DB_HASH, /* Database access method */
        flags, /* Open flags */
        0); /* File mode (using defaults) */
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB Open Error\n");
        return -1;
    }

    std::cout << "Listening\n";
    if ((TcpListenPort = OpenTcpListenPort(2222)) == NULL)  // Open UDP Network port
    {
        std::cout << "OpenTcpListenPortFailed\n";
        return(-1);
    }
    clilen = sizeof(cli_addr);
    
    //////////
    FD_SET WriteSet;
    FD_SET ReadSet;

    long long max_search_time = 0;
    while (TRUE)
    {
        int Total;
        int nfsd = static_cast<int>(TcpListenPort->ListenFd);
        FD_ZERO(&ReadSet);
        FD_ZERO(&WriteSet);
        FD_SET(TcpListenPort->ListenFd, &ReadSet);
        for (auto& connected_fd : connected_ports) {
            FD_SET(connected_fd->ConnectedFd, &ReadSet);
            nfsd = max(nfsd, static_cast<int>(connected_fd->ConnectedFd));
        }

        printf("Trying select\n");
        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        //if (Total = select(nfsd + 1, &ReadSet, NULL, NULL, &timeout) == SOCKET_ERROR)
        /* No use timeout */
        if (Total = select(nfsd + 1, &ReadSet, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("select() returned with error %d\n", WSAGetLastError());
            return(-1);
        }
        else
        {
            printf("select is OK : Total : %d\n", Total);
        }

        if (FD_ISSET(TcpListenPort->ListenFd, &ReadSet)) {
            if ((TcpConnectedPort = AcceptTcpConnection(TcpListenPort, &cli_addr, &clilen)) == NULL)
            {
                printf("AcceptTcpConnection Failed\n");
                return(-1);
            }
            printf("connected\n");
            connected_ports.insert(shared_ptr<TTcpConnectedPort>(TcpConnectedPort));
            Total--;
        }
        // FIXME : select return 0 when event occurs. But 0 means timeout. Need to figure out.
        //if(Total != 0) { 
            for (auto& connected_fd : connected_ports) {
                if (FD_ISSET(connected_fd->ConnectedFd, &ReadSet)) {
                    //Total--;
                    if (ReadDataTcp(connected_fd.get(), (unsigned char*)&PlateStringLength, sizeof(PlateStringLength)) != sizeof(PlateStringLength))
                    {
                        printf("ReadDataTcp 1 error - close socket\n");
                        closesocket(connected_fd->ConnectedFd);
                        connected_ports.erase(connected_fd);
                        break;
                    }
                    PlateStringLength = ntohs(PlateStringLength);
                    if (PlateStringLength > sizeof(PlateString))
                    {
                        printf("Plate string length  error\n");
                        continue;
                    }
                    if (ReadDataTcp(connected_fd.get(), (unsigned char*)&PlateString, PlateStringLength) != PlateStringLength)
                    {
                        printf("ReadDataTcp 2 error\n");
                        continue;
                    }
                    printf("Plate is : %s\n", PlateString);
                    auto start_time = std::chrono::milliseconds(GetTickCount64());
                    if (partialMatch(dbp, PlateString, DBRecord, sizeof(DBRecord)))
                    {
                        int sendlength = (int)(strlen((char*)DBRecord) + 1);
                        short SendMsgHdr = ntohs(sendlength);
                        if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendMsgHdr))
                            printf("WriteDataTcp %lld\n", result);
                        if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)DBRecord, sendlength)) != sendlength)
                            printf("WriteDataTcp %lld\n", result);
                        printf("sent ->%s\n", (char*)DBRecord);
                    }
#if 0
                    else {
                        key.flags = DB_DBT_USERMEM | DB_DBT_PARTIAL;
                        key.dlen = 4U;
                        key.doff = 0;
                        if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND) {
                            printf("----------------- FIND Partial!!!\n");
                            int sendlength = (int)(strlen((char*)data.data) + 1);
                            short SendMsgHdr = ntohs(sendlength);
                            if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendMsgHdr))
                                printf("WriteDataTcp %lld\n", result);
                            if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)data.data, sendlength)) != sendlength)
                                printf("WriteDataTcp %lld\n", result);
                            printf("sent ->%s\n", (char*)data.data);
                        }
                    }
#endif
                    //Sleep(10);
                    auto search_time = (std::chrono::milliseconds(GetTickCount64()) - start_time).count();
                    
                    max_search_time = max(max_search_time, search_time);
                    cout << ">>>>>>>>>>>> DB search time :" << search_time << " (max : " << max_search_time << ")" << endl;
                }
            //}

        }

    }
    //////////
#if 0 // TODO : Delete
    if ((TcpConnectedPort = AcceptTcpConnection(TcpListenPort, &cli_addr, &clilen)) == NULL)
    {
        printf("AcceptTcpConnection Failed\n");
        return(-1);
    }
    printf("connected\n");
    while (1)
    {
        if (ReadDataTcp(TcpConnectedPort, (unsigned char*)&PlateStringLength, sizeof(PlateStringLength)) != sizeof(PlateStringLength))
        {
            printf("ReadDataTcp 1 error\n");
            return(-1);
        }
        PlateStringLength = ntohs(PlateStringLength);
        if (PlateStringLength > sizeof(PlateString))
        {
            printf("Plate string length  error\n");
            return(-1);
        }
        if (ReadDataTcp(TcpConnectedPort, (unsigned char*)&PlateString, PlateStringLength) != PlateStringLength)
        {
            printf("ReadDataTcp 2 error\n");
            return(-1);
        }
        printf("Plate is : %s\n", PlateString);

        /* Zero out the DBTs before using them. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
        key.data = PlateString;
        key.size = (u_int32_t) (strlen(PlateString)+1);
        data.data = DBRecord;
        data.ulen = sizeof(DBRecord);
        data.flags = DB_DBT_USERMEM;
        if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND)
        {
            int sendlength = (int)(strlen((char*)data.data) + 1);
            short SendMsgHdr=ntohs(sendlength);
            if ((result = WriteDataTcp(TcpConnectedPort, (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendMsgHdr))
                printf("WriteDataTcp %lld\n", result);
            if ((result = WriteDataTcp(TcpConnectedPort, (unsigned char*)data.data, sendlength)) != sendlength)
                printf("WriteDataTcp %lld\n", result);
            printf("sent ->%s\n", (char*)data.data);
        }
       // else printf("not Found\n");



    }
#endif

}



