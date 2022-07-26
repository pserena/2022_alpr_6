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
#include <fstream>

#include <iostream>
#include <cstdlib>
#include <signal.h>

#include "RequestHandler.h"
#include "AesManager.h"

using namespace std;


RequestHandler rh;

void signal_callback_handler(int signum) {
    cout << "Caught signal " << signum << endl;
    rh.fileWriteInformation();
    exit(signum);
}

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
    unsigned short SendMsgHdr = ntohs((u_short)response.length());
    //cout << "--------- length : " << response.length() << "----------" << endl;
    {
        lock_guard<mutex> l(send_lock_);
        //cout << response.length() << endl;
        WriteDataTcp(tcp_connected_port.get(), (const unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr));
        //cout << endl << response << endl;
        WriteDataTcp(tcp_connected_port.get(), (const unsigned char*)response.c_str(), response.length());
    }
}

int main()
{
    TTcpListenPort* TcpListenPort;
    TTcpConnectedPort* TcpConnectedPort;
    unordered_set<shared_ptr<TTcpConnectedPort>> connected_ports;
    struct sockaddr_in cli_addr;
    socklen_t          clilen;
    bool NeedStringLength = true;
    unsigned short DataStringLength;
    char DataString[4096];
    ofstream log_output_;
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
    log_output_.open("6team.server.log", std::ofstream::out);
    std::cout << "Start Server" << endl;
    log_output_ << GetTickCount64() << ": " << "Start Server" << endl;
    if ((TcpListenPort = OpenTcpListenPort(2222)) == NULL)  // Open UDP Network port
    {
        std::cout << "OpenTcpListenPortFailed\n";
        log_output_ << GetTickCount64() << ": " << "TCP Listen Port Failed " << endl;
        return(-1);
    }
    clilen = sizeof(cli_addr);
    
    //////////
    FD_SET WriteSet;
    FD_SET ReadSet;

    long long max_search_time = 0;
	
	signal(SIGINT, signal_callback_handler);
	
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

        //printf("Trying select\n");

        //if (Total = select(nfsd + 1, &ReadSet, NULL, NULL, &timeout) == SOCKET_ERROR)
        /* No use timeout */
        if (Total = select(nfsd + 1, &ReadSet, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("select() returned with error %d\n", WSAGetLastError());
            return(-1);
        }
        else
        {
            //printf("select is OK : Total : %d\n", Total);
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
            
            log_output_ << GetTickCount64() << ": " << "connected. " << ipbuf << "( PORT:" << cli_addr.sin_port << ")"  <<  endl;
            
            connected_ports.insert(shared_ptr<TTcpConnectedPort>(TcpConnectedPort));
            rh.connect(TcpConnectedPort->ConnectedFd);
        }

		for (auto& connected_fd : connected_ports) {
			if (FD_ISSET(connected_fd->ConnectedFd, &ReadSet)) {
				if (ReadDataTcp(connected_fd.get(), (unsigned char*)&DataStringLength, sizeof(DataStringLength)) != sizeof(DataStringLength))
				{
					printf("ReadDataTcp 1 error - close socket\n");
                    log_output_ << GetTickCount64() << ": " << inet_ntoa(cli_addr.sin_addr) << "- ReadDataTcp 1 error - close socket" << endl;
					closesocket(connected_fd->ConnectedFd);
                    rh.disconnect(connected_fd->ConnectedFd);
					connected_ports.erase(connected_fd);
					break;
				}
                

				unsigned short data_length = ntohs(DataStringLength);
                //cout << "DataStringLength : " << data_length << endl;
				if (data_length > sizeof(DataString))
				{
					printf("Data string length error : %d (%x)\n", data_length, data_length);
                    log_output_ << GetTickCount64() << ": " << inet_ntoa(cli_addr.sin_addr) << "Data string length error :" << data_length << endl;
                    return 0;
					continue;
				}

                auto read_size = ReadDataTcp(connected_fd.get(), (unsigned char*)&DataString, data_length);
				if (read_size != data_length)
				{
					printf("ReadDataTcp 2 error %lld vs %d\n", read_size, data_length);
                    log_output_ << GetTickCount64() << ": " << inet_ntoa(cli_addr.sin_addr) << "ReadDataTcp 2 error  :" << data_length << endl;
					continue;
				}
				//printf("Data is : %s\n", DataString);

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



