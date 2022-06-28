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


using namespace std;

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
    DBT key, data;
    DB* dbp; /* DB structure handle */
    u_int32_t flags; /* database open flags */
    int ret; /* function return value */
    ssize_t result;

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
                    /* Zero out the DBTs before using them. */
                    memset(&key, 0, sizeof(DBT));
                    memset(&data, 0, sizeof(DBT));
                    key.data = PlateString;
                    key.size = (u_int32_t)(strlen(PlateString) + 1);
                    data.data = DBRecord;
                    data.ulen = sizeof(DBRecord);
                    data.flags = DB_DBT_USERMEM;
                    if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND)
                    {
                        int sendlength = (int)(strlen((char*)data.data) + 1);
                        short SendMsgHdr = ntohs(sendlength);
                        if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendMsgHdr))
                            printf("WriteDataTcp %lld\n", result);
                        if ((result = WriteDataTcp(connected_fd.get(), (unsigned char*)data.data, sendlength)) != sendlength)
                            printf("WriteDataTcp %lld\n", result);
                        printf("sent ->%s\n", (char*)data.data);
                    }
                    auto end_time = std::chrono::milliseconds(GetTickCount64());
                    cout << "DB search time :" << (end_time - start_time).count() << endl;
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



