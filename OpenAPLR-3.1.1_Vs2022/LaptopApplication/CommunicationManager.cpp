#include "LapTopModules.h"

#include <iostream>
#include "NetworkTCP.h"
#include "json.hpp"

TTcpConnectedPort* TcpConnectedPort;
//enum ResponseMode { ReadingHeader, ReadingMsg };

using namespace client;
client::ResponseMode GetResponseMode = client::ResponseMode::ReadingHeader;
static void GetResponses(char* data);
size_t RespHdrNumBytes;
unsigned int BytesInResponseBuffer = 0;
ssize_t BytesNeeded = sizeof(RespHdrNumBytes);

CommunicationManager::CommunicationManager(void)
{
    //networkConnect();
}
CommunicationManager::~CommunicationManager(void)
{
}

int CommunicationManager::networkConnect(void) {
    if ((TcpConnectedPort = OpenTcpConnection("127.0.0.1", "2222")) == NULL) {
        std::cout << "Connection Failed" << std::endl;
        return (-1);
    } else
        std::cout << "Connected" << std::endl;
    return 0;
}

int CommunicationManager::networkConnectClose(void) {
    CloseTcpConnectedPort(&TcpConnectedPort);
    return 0;
}

int CommunicationManager::retryNetworkConnect(void) {
    //networkConnectClose();
    networkConnect();
    return 0;
}

int CommunicationManager::sendCommunicationData(unsigned char* data) {
    ssize_t result;
    unsigned short SendMsgHdr, SendPlateStringLength;
    SendPlateStringLength = (unsigned short)strlen((char*)data) + 1;
    SendMsgHdr = htons(SendPlateStringLength);
    if ((result = (int)WriteDataTcp(TcpConnectedPort, (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) != sizeof(SendPlateStringLength))
        printf("WriteDataTcp %d\n", result);
    if ((result = (int)WriteDataTcp(TcpConnectedPort, (unsigned char*)data, SendPlateStringLength)) != SendPlateStringLength)
        printf("WriteDataTcp %d\n", result);
    printf("sent ->%s\n", data);
    return 0;
}

int CommunicationManager::receiveCommunicationData(char* data)
{
    GetResponses(data);
    return 0;
}

static void GetResponses(char* data)
{
    char* ResponseBuffer = data;
    ssize_t BytesRead;
    ssize_t BytesOnSocket = 0;
    while ((BytesOnSocket = BytesAvailableTcp(TcpConnectedPort)) > 0)
    {
        if (BytesOnSocket < 0) return;
        if (BytesOnSocket > BytesNeeded) BytesOnSocket = BytesNeeded;
        BytesRead = ReadDataTcp(TcpConnectedPort, (unsigned char*)&ResponseBuffer[BytesInResponseBuffer], BytesOnSocket);
        if (BytesRead <= 0)
        {
            printf("Read Response Error - Closing Socket\n");
            CloseTcpConnectedPort(&TcpConnectedPort);
        }
        BytesInResponseBuffer += BytesRead;

        if (BytesInResponseBuffer == BytesNeeded)
        {
            if (GetResponseMode == ResponseMode::ReadingHeader)
            {
                memcpy(&RespHdrNumBytes, ResponseBuffer, sizeof(RespHdrNumBytes));
                RespHdrNumBytes = ntohs(RespHdrNumBytes);
                GetResponseMode = ResponseMode::ReadingMsg;
                BytesNeeded = RespHdrNumBytes;
                BytesInResponseBuffer = 0;
            }
            else if (GetResponseMode == ResponseMode::ReadingMsg)
            {
                printf("Response %s\n", ResponseBuffer);
                GetResponseMode = ResponseMode::ReadingHeader;
                BytesInResponseBuffer = 0;
                BytesNeeded = sizeof(RespHdrNumBytes);
            }
        }
    }
    if (BytesOnSocket < 0)
    {
        printf("Read Response Error - Closing Socket\n");
        CloseTcpConnectedPort(&TcpConnectedPort);
    }
}

int CommunicationManager::authenticate(string strID, string strPw) {
	return 0;
}