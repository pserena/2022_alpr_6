#include "LapTopModules.h"

#include <iostream>
#include "NetworkTCP.h"
#include "json.hpp"

TTcpConnectedPort* TcpConnectedPort;

using namespace client;

client::ResponseMode GetResponseMode = client::ResponseMode::ReadingHeader;
static int GetResponses(char* data);
unsigned short RespHdrNumBytes;
unsigned int BytesInResponseBuffer = 0;
ssize_t BytesNeeded = sizeof(RespHdrNumBytes);

static string userID = "";
static string userPass = "";
static int reconnectThread(CommunicationManager* commMan);

CommunicationManager::CommunicationManager(const string& ipaddr) : ipaddr_(ipaddr)
{}

CommunicationManager::~CommunicationManager(void)
{
}

int CommunicationManager::networkConnect(void) {
    const string& ipaddr = ipaddr_.empty() ? "127.0.0.1" : ipaddr_;
    if ((TcpConnectedPort = OpenTcpConnection(ipaddr.c_str(), "2222")) == NULL) { 
        std::cout << "Connection Failed to " << ipaddr << std::endl;
        retryNetworkConnectSave(true);
        return (-1);
    }
    else {
        retryNetworkConnectSave(false);
        std::cout << "Connected to " << ipaddr << std::endl;
    }
    return 0;
}

int CommunicationManager::networkConnectClose(void) {
    CloseTcpConnectedPort(&TcpConnectedPort);
    return 0;
}

int CommunicationManager::retryNetworkConnectSave(bool save) {
    if (save != saveRetry) {
        printf("retryNetworkConnectSave 11 %d\n", save);
        saveRetry = save;
    }
    return 0;
}

int CommunicationManager::retryNetworkConnect(void) {
    printf("retryNetworkConnect start \n");
    networkConnectClose();
    if (networkConnect() >=0) {
        if (!userID.empty()) {
            authenticate(userID, userPass);
        }
    }
    return 0;
}

int CommunicationManager::sendCommunicationData(unsigned char* data) {
    ssize_t result;
    unsigned short SendMsgHdr, SendPlateStringLength;
    SendPlateStringLength = (unsigned short)strlen((char*)data) + 1;
    SendMsgHdr = htons(SendPlateStringLength);
    if (TcpConnectedPort == NULL || saveRetry) {
        //printf("skip sendCommunicationData :: %d\n", saveRetry);
        retryNetworkConnectSave(true);
        return -1;
    }
    else if ((result = (int)WriteDataTcp(TcpConnectedPort, (unsigned char*)&SendMsgHdr, sizeof(SendMsgHdr))) == sizeof(SendPlateStringLength)) {
        if ((result = (int)WriteDataTcp(TcpConnectedPort, (unsigned char*)data, SendPlateStringLength)) != SendPlateStringLength) {
            printf("WriteDataTcp 11 :: %d\n", result);
            retryNetworkConnectSave(true);
            return -1;
        }
        else {
            printf("sent ->%s\n", data);
        }
    } else {
        printf("WriteDataTcp 22 :: %d\n", result);
        retryNetworkConnectSave(true);
        return -1;
    }
    return 0;
}

int CommunicationManager::receiveAuthenticateData(char* data)
{
    if (TcpConnectedPort == NULL) {
        retryNetworkConnectSave(true);
        return -1;
    }
    int result = GetResponses(data);
    if (result < 0) {
        retryNetworkConnectSave(true);
        return -1;
    }
    return 0;
}

int CommunicationManager::receiveCommunicationData(char* data)
{
    if (TcpConnectedPort == NULL) {
        retryNetworkConnectSave(true);
        return -1;
    }
    else {
        int result = GetResponses(data);
        if (result < 0) {
            retryNetworkConnectSave(true);
            return -1;
        }
#if 0
        else {
            //cout << data << end;
            json responseJson = json::parse(data);
            if (responseJson["request_type"] == "query") {
                if (responseJson["response_code"] == 200 && responseJson["response"]["numFound"] != 0)
                {
                    clock_t recTime = time(NULL);
                    vector<string> vecPlateNum;
                    int num = responseJson["response"]["docs"].size();
                    for (int i = 0; i < num; i++) {
                        string plate_number = responseJson["response"]["docs"].at(i)["plate_number"].at(0).get<string>();
                        printf("################## %s\n", plate_number);
                        vecPlateNum.push_back(plate_number);
                    }
                    ///////////////////////////////////////
                    // ť�� "RESPONSE recTime vecPlateNum" �������� ������ �˴ϴ�.
                }
            }
        }
#endif
    }
    return 0;
}

static int GetResponses(char* data)
{
    unsigned short DataStringLength;
    //cout << "Get Response" << endl;
    if (ReadDataTcp(TcpConnectedPort, (unsigned char*)&DataStringLength, sizeof(DataStringLength)) != sizeof(DataStringLength)) {
        //cout << "Read Response Error - Closing Socket" << endl;
        return -1;
    }
    unsigned short data_length = ntohs(DataStringLength);
    //cout << "length : " << data_length << endl;
    auto read_size = ReadDataTcp(TcpConnectedPort, (unsigned char*)data, data_length);
    if (read_size != data_length)
    {
        printf("ReadDataTcp 2 error %lld vs %d\n", read_size, data_length);
        return -1;
    }
    //cout << data << endl;
    return 0;
#if 0
    char* ResponseBuffer = data;
    ssize_t BytesRead;
    ssize_t BytesOnSocket = 0;
    while ((BytesOnSocket = BytesAvailableTcp(TcpConnectedPort)) > 0)
    {
        if (BytesOnSocket < 0) return -1;
        if (BytesOnSocket > BytesNeeded) BytesOnSocket = BytesNeeded;
        BytesRead = ReadDataTcp(TcpConnectedPort, (unsigned char*)&ResponseBuffer[BytesInResponseBuffer], BytesOnSocket);
        if (BytesRead <= 0)
        {
            printf("Read Response Error - Closing Socket\n");
            //CloseTcpConnectedPort(&TcpConnectedPort);
            return -1;
        }
        else {
            BytesInResponseBuffer += BytesRead;
        }

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
                //printf("Response %s\n", ResponseBuffer);
                //printf("Response %s\n", "1111");
                GetResponseMode = ResponseMode::ReadingHeader;
                BytesInResponseBuffer = 0;
                BytesNeeded = sizeof(RespHdrNumBytes);
            }
        }
    }

    if (BytesOnSocket < 0)
    {
        printf("Read Response Error - Closing Socket\n");
        //CloseTcpConnectedPort(&TcpConnectedPort);
        return -1;
    }
    return 0;
#endif
}

int CommunicationManager::authenticate(string strID, string strPw) {
    reconnectThreadStart();
    if (!strID.empty()) {
        userID = strID;
        userPass = strPw;
    }
    auto jsonMessageLogin = R"(
        {
            "request_type": "login"
        }
    )"_json;
    jsonMessageLogin["user_id"] = strID.c_str(); //"daniel";
    jsonMessageLogin["user_password"] = strPw.c_str(); //"1qaz2wsx";
    string messageLogin = jsonMessageLogin.dump();
    printf("sendAuthenticate %s\n", messageLogin.c_str());
    sendCommunicationData((unsigned char* )messageLogin.c_str());

	return 0;
}

int CommunicationManager::sendRecognizedInfo(string rs, int puid) {
    auto jsonMessage = R"(
        {
            "request_type": "query"
        }
    )"_json;
    jsonMessage["plate_number"] = rs.c_str();
    jsonMessage["plate_uid"] = to_string(puid);
    string messageRecognized = jsonMessage.dump();
    //printf("sendRecognizedInfo %s\n", messageRecognized.c_str());
    int result = sendCommunicationData((unsigned char*)messageRecognized.c_str());
    if (result >= 0) {
        clock_t recTime = time(NULL);
        vector<string> vecPlateNum;
        vecPlateNum.push_back(rs);
        ///////////////////////////////////////
        // ť�� "REQUEST recTime vecPlateNum" �������� ������ �˴ϴ�.
    }
    return 0;
}

void CommunicationManager::reconnectThreadStart(void) {
    static bool startThread = false;
    if (!startThread) {
        timer_start(reconnectThread, 100);
        startThread = true;
    }
}

void CommunicationManager::timer_start(std::function<void(CommunicationManager*)> func, unsigned int interval)
{
    std::thread([func, this, interval]() {
        while (true)
        {
            func(this);
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
        }).detach();
}

static int reconnectThread(CommunicationManager* commMan)
{
    if (commMan->saveRetry) {
        commMan->retryNetworkConnect();
    }
    //printf("receiveThreadt\n");
    return 0;
}