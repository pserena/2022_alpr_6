// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SOLRDB

#include <iostream>
#include <string.h>
#include "NetworkTCP.h"
#include <Windows.h>
#ifndef SOLRDB
#include <db.h> 
#endif
#include <unordered_set>
#include <memory>
#include <mutex>

#include <stdio.h>
#include <thread>

#include "RequestHandler.h"

using namespace std;

#ifndef SOLRDB

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
#if 0
    for (char c = '0'; c <= '9'; c++) {
        if (doPartitionSearch(dbp, plate + c, out, out_len))
            return true;
    }
    for (char c = 'A'; c <= 'Z'; c++) {
        if (doPartitionSearch(dbp, plate + c, out, out_len))
            return true;
    }
#endif
     return false;
}

bool partialMatch(DB* dbp, char* plate, char* out, u_int32_t out_len) {
    /* Zero out the DBTs before using them. */
    return doPartitionSearch(dbp, string(plate), out, out_len);
}
#endif


mutex send_lock_;
void sendResponse(shared_ptr<TTcpConnectedPort> tcp_connected_port, string response) {
    char buf[8192];
    size_t SendMsgHdr = ntohs(response.length());
    strcpy_s(buf, response.c_str());
    cout << "--------- length : " << response.length() << "----------" << endl;
    //cout << response << endl;
    {
        lock_guard<mutex> l(send_lock_);
        WriteDataTcp(tcp_connected_port.get(), (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr));
        WriteDataTcp(tcp_connected_port.get(), (unsigned char*)buf, response.length());
        cout << "[sendResponse] " << buf << endl;
    }
}

#if 0
void TestJson()
{
    json j;
    j["pi"] = 3.141;
    j["happy"] = true;
    j["name"] = "Niels";
    j["nothing"] = nullptr;
    j["answer"]["everything"] = 42;
    j["list"] = { 1, 0, 2 };
    j["object"] = { {"currency", "USD"}, {"value", 42.99} };
    cout << j << endl;

    json j2 = {
      {"pi", 3.141},
      {"happy", true},
      {"name", "Niels"},
      {"nothing", nullptr},
      {"answer", {
        {"everything", 42}
      }},
      {"list", {1, 0, 2}},
      {"object", {
        {"currency", "USD"},
        {"value", 42.99}
      }}
    };

    cout << j2 << endl;

    cout << "!!!!!!!!" << j2["name"] << endl;

    assert(j == j2);

    json j11 = "{ \"happy\": true, \"pi\": 3.141 }"_json;
    auto j12 = R"(
        {
            "happy": true,
            "pi": 3.141
        }
    )"_json;
    auto j13 = json::parse("{ \"happy\": true, \"pi\": 3.141 }");

    assert(j11 == j12 && j == j13);

    string s = j.dump();
    cout << "serialization: " << s << endl;

    cout << "serialization with pretty printing: " << j.dump(4) << endl;
}
#endif
int main()
{
    //TestJson();

    TTcpListenPort* TcpListenPort;
    TTcpConnectedPort* TcpConnectedPort;
    unordered_set<shared_ptr<TTcpConnectedPort>> connected_ports;
    struct sockaddr_in cli_addr;
    socklen_t          clilen;
    bool NeedStringLength = true;
    unsigned short DataStringLength;
    char DataString[1024];
#ifndef SOLRDB
    char DBRecord[2048];
    DB* dbp; /* DB structure handle */
    u_int32_t flags; /* database open flags */
    int ret; /* function return value */
    ssize_t result;

#endif

#ifndef SOLRDB
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
#endif

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
    RequestHandler rh;
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

            printf("connected.\n");
            char ipbuf[INET_ADDRSTRLEN] = { 0, };
            memset(ipbuf, 0x00, sizeof(char)* INET_ADDRSTRLEN);
            strcpy(ipbuf, inet_ntoa(cli_addr.sin_addr));
            printf("IP address : %s\n", ipbuf);
            printf("Port : %d\n", ntohs(cli_addr.sin_port));
            connected_ports.insert(shared_ptr<TTcpConnectedPort>(TcpConnectedPort));
            rh.connect(TcpConnectedPort->ConnectedFd);
        }

		for (auto& connected_fd : connected_ports) {
			if (FD_ISSET(connected_fd->ConnectedFd, &ReadSet)) {
				if (ReadDataTcp(connected_fd.get(), (unsigned char*)&DataStringLength, sizeof(DataStringLength)) != sizeof(DataStringLength))
				{
					printf("ReadDataTcp 1 error - close socket\n");
					closesocket(connected_fd->ConnectedFd);
                    rh.disconnect(connected_fd->ConnectedFd);
					connected_ports.erase(connected_fd);
					break;
				}
				DataStringLength = ntohs(DataStringLength);
				if (DataStringLength > sizeof(DataString))
				{
					printf("Data string length error\n");
					continue;
				}
				if (ReadDataTcp(connected_fd.get(), (unsigned char*)&DataString, DataStringLength) != DataStringLength)
				{
					printf("ReadDataTcp 2 error\n");
					continue;
				}
				printf("Data is : %s\n", DataString);

#ifdef SOLRDB
                function<void(string)> callback = bind(&sendResponse, connected_fd, placeholders::_1);
                /* TODO : Test Code for Solr DB */
                rh.handle(connected_fd->ConnectedFd, DataString, move(callback));
#else
				if (partialMatch(dbp, DataString, DBRecord, sizeof(DBRecord)))
				{
					int sendlength = (int)(strlen((char*)DBRecord) + 1);
					short SendMsgHdr = ntohs(sendlength);
					if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendMsgHdr))
						printf("WriteDataTcp %lld\n", result);
					if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)DBRecord, sendlength)) != sendlength)
						printf("WriteDataTcp %lld\n", result);
					printf("sent ->%s\n", (char*)DBRecord);
				}
#endif
			}
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
        if (ReadDataTcp(TcpConnectedPort, (unsigned char*)&DataStringLength, sizeof(DataStringLength)) != sizeof(DataStringLength))
        {
            printf("ReadDataTcp 1 error\n");
            return(-1);
        }
        DataStringLength = ntohs(DataStringLength);
        if (DataStringLength > sizeof(DataString))
        {
            printf("Data string length  error\n");
            return(-1);
        }
        if (ReadDataTcp(TcpConnectedPort, (unsigned char*)&DataString, DataStringLength) != DataStringLength)
        {
            printf("ReadDataTcp 2 error\n");
            return(-1);
        }
        printf("Data is : %s\n", DataString);

        /* Zero out the DBTs before using them. */
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));
        key.data = DataString;
        key.size = (u_int32_t) (strlen(DataString)+1);
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



