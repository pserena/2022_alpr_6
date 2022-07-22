
#include <iostream>
#include <windows.h>
#include <tchar.h>

#include "LaptopModules.h"
#include "opencv2/opencv.hpp"
#include "DeviceEnumerator.h"
#include "json.hpp"

using namespace cv;
using namespace std;
using namespace client;
using json = nlohmann::json;

#define VI_WIDTH 520
#define VI_HEIGHT 480

UIManager::UIManager()
{
    vimg = Mat(100, VI_WIDTH, CV_8UC3, Scalar(255, 255, 255));
    vtext = Mat(140, VI_WIDTH, CV_8UC3, Scalar(255, 255, 255));
    aimg = Mat(100, VI_WIDTH, CV_8UC3, Scalar(255, 255, 255));
    atext = Mat(140, VI_WIDTH, CV_8UC3, Scalar(255, 255, 255));
    vinfo = Mat(VI_HEIGHT, VI_WIDTH, CV_8UC3, Scalar(255, 255, 255));
    connection_lost = false;
}

static bool getconchar(KEY_EVENT_RECORD& krec)
{
    DWORD cc;
    INPUT_RECORD irec;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

    if (h == NULL)
    {
        return false; // console not found
    }

    for (; ; )
    {
        ReadConsoleInput(h, &irec, 1, &cc);
        if (irec.EventType == KEY_EVENT
            && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown
            )//&& ! ((KEY_EVENT_RECORD&)irec.Event).wRepeatCount )
        {
            krec = (KEY_EVENT_RECORD&)irec.Event;
            return true;
        }
    }
    return false; //future ????
}

Mode UIManager::GetVideoMode(void)
{
    KEY_EVENT_RECORD key;
    Mode mode = Mode::mNone;
    do
    {
        std::cout << "Select PlayBack File, Live Video or Image File" << std::endl;
        std::cout << "1 - PlayBack File" << std::endl;
        std::cout << "2 - Live Video" << std::endl;
        std::cout << "3 - Image File" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        else if (key.uChar.AsciiChar == '1') mode = Mode::mPlayback_Video;
        else if (key.uChar.AsciiChar == '2') mode = Mode::mLive_Video;
        else if (key.uChar.AsciiChar == '3') mode = Mode::mImage_File;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (mode == Mode::mNone);
    return(mode);
}

int UIManager::GetVideoDevice(void)
{
    int deviceID = -1;
    KEY_EVENT_RECORD key;
    int numdev;
    DeviceEnumerator de;
    std::map<int, Device> devices = de.getVideoDevicesMap();

    int* deviceid = new int[devices.size()];
    do {
        numdev = 0;
        std::cout << "Select video Device" << std::endl;
        for (auto const& device : devices)
        {
            deviceid[numdev] = device.first;
            std::cout << numdev + 1 << " - " << device.second.deviceName << std::endl;
            numdev++;
        }
        std::cout << "E - exit" << std::endl;
        getconchar(key);
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        int value = static_cast<int>(key.uChar.AsciiChar) - 48;
        if ((value >= 1) && value <= numdev) deviceID = deviceid[value - 1];
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (deviceID == -1);
    delete[] deviceid;
    return(deviceID);
}

VideoResolution UIManager::GetVideoResolution(void)
{
    VideoResolution vres = VideoResolution::rNone;
    KEY_EVENT_RECORD key;
    do
    {
        std::cout << "Select Video Resolution" << std::endl;
        std::cout << "1 - 640 x 480" << std::endl;
        std::cout << "2 - 1280 x 720" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        else if (key.uChar.AsciiChar == '1') vres = VideoResolution::r640X480;
        else if (key.uChar.AsciiChar == '2') vres = VideoResolution::r1280X720;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (vres == VideoResolution::rNone);

    return(vres);
}

bool UIManager::GetFileName(Mode mode, char filename[MAX_PATH])
{
    TCHAR CWD[MAX_PATH];
    bool retval = true;
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];
    ZeroMemory(&szFile, sizeof(szFile));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
    if (mode == Mode::mImage_File) ofn.lpstrFilter = _T("Image Files\0*.png;*.jpg;*.tif;*.bmp;*.jpeg;*.gif\0Any File\0*.*\0");
    else if (mode == Mode::mPlayback_Video) ofn.lpstrFilter = _T("Video Files\0*.avi;*.mp4;*.webm;*.flv;*.mjpg;*.mjpeg\0Any File\0*.*\0");
    else ofn.lpstrFilter = _T("Text Files\0*.txt\0Any File\0*.*\0");
    ofn.lpstrFile = LPWSTR(szFile);
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Select a File, to Processs");
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    GetCurrentDirectory(MAX_PATH, CWD);
    if (GetOpenFileName(&ofn))
    {
        size_t output_size;
        wcstombs_s(&output_size, filename, MAX_PATH, ofn.lpstrFile, MAX_PATH);
    }
    else
    {
        // All this stuff below is to tell you exactly how you messed up above. 
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        switch (CommDlgExtendedError())
        {
        case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
        case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
        case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
        case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
        case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
        case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
        case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
        case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
        case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
        case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
        case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
        case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
        case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
        case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
        case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
        default: std::cout << "You cancelled.\n";
            retval = false;
        }
    }
    SetCurrentDirectory(CWD);

    if (retval) std::cout << "Filename is " << filename << std::endl;
    return(retval);
}

VideoSaveMode UIManager::GetVideoSaveMode(void)
{
    VideoSaveMode videosavemode = VideoSaveMode::vNone;
    KEY_EVENT_RECORD key;
    do
    {
        std::cout << "Select Video Save Mode" << std::endl;
        std::cout << "1 - No Save" << std::endl;
        std::cout << "2 - Save" << std::endl;
        std::cout << "3 - Save With No ALPR" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) exit(0);
        else if (key.uChar.AsciiChar == '1') videosavemode = VideoSaveMode::vNoSave;
        else if (key.uChar.AsciiChar == '2') videosavemode = VideoSaveMode::vSave;
        else if (key.uChar.AsciiChar == '3') videosavemode = VideoSaveMode::vSaveWithNoALPR;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (videosavemode == VideoSaveMode::vNone);

    return(videosavemode);
}

void UIManager::PrintErrMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

static void puttext_info(Mat plate, const char* d1, const char* d2, const char* d3,
    const char* d4, const char* d5, int x, int y)
{   
    cv::putText(plate, d1,
        cv::Point(x, y + 10),
        FONT_HERSHEY_DUPLEX, 0.8,
        Scalar(102, 0, 0), 1, LINE_AA, false
    );

    cv::putText(plate, d2,
        cv::Point(x - 20, y + 30),
        FONT_HERSHEY_DUPLEX, 0.4,
        Scalar(0, 0, 255), 1, LINE_AA, false
    );
    
    cv::putText(plate, d3,
        cv::Point(x + 130, y),
        FONT_HERSHEY_DUPLEX, 0.4,
        Scalar(0, 51, 0), 0, LINE_AA, false
    );

    cv::putText(plate, d4,
        cv::Point(x + 130, y + 15),
        FONT_HERSHEY_DUPLEX, 0.4,
        Scalar(0, 51, 0), 0, LINE_AA, false
    );

    cv::putText(plate, d5,
        cv::Point(x + 130, y + 30),
        FONT_HERSHEY_DUPLEX, 0.4,
        Scalar(102, 0, 102), 0, LINE_AA, false
    );
}

void UIManager::UpdateVinfo(string plate_number, int puid, Mat pimag, json jsonRetPlateInfo, int nError)
{
    connection_lost = nError;

    if (nError == 1) {
        // network disconnect
        printf("network disconnect error UpdateVinfo \n");
        return;
    }

    json docs = jsonRetPlateInfo["docs"];

    if (docs.size() == 0)
        return;

    Mat info(vtext.size(), CV_8UC3, Scalar(255, 255, 255));
    Mat ainfo(atext.size(), CV_8UC3, Scalar(255, 255, 255));
    int alert_count = 0;
    int vehicle_count = 0;

    for (int i = 0; i < docs.size(); ++i)
    {
        string status = docs.at(i)["status"].at(0).get<std::string>();
        string plate_num = docs.at(i)["plate_number"].at(0).get<std::string>();

        int vehicle_year = docs.at(i)["vehicle_year"].at(0).get<int>();
        string vehicle_make = docs.at(i)["vehicle_make"].at(0).get<std::string>();
        string vehicle_model = docs.at(i)["vehicle_model"].at(0).get<std::string>();
        string vehicle_color = docs.at(i)["vehicle_color"].at(0).get<std::string>();

        char vehicle_info[256];
        sprintf_s(vehicle_info, sizeof(vehicle_info), "%d %s %s %s",
            vehicle_year,
            vehicle_make.c_str(),
            vehicle_model.c_str(),
            vehicle_color.c_str()
        );

        if (strcmp(status.c_str(), "No Wants / Warrants")) {
            string reg_expiration = docs.at(i)["reg_expiration"].at(0).get<std::string>();
            string owner_name = docs.at(i)["owner_name"].at(0).get<std::string>();
            string owner_birthdate = docs.at(i)["owner_birthdate"].at(0).get<std::string>();
            string owner_address_1 = docs.at(i)["owner_address_1"].at(0).get<std::string>();
            string owner_address_2 = docs.at(i)["owner_address_2"].at(0).get<std::string>();

            char aligned_status[20];
            string space = "";
            status.erase(remove(status.begin(), status.end(), ' '));
            int space_length = (sizeof(aligned_status) - status.length()) / 2;
            if (space_length > 0)
                space.append(space_length, ' ');

            sprintf_s(aligned_status, sizeof(aligned_status), "%s %s", space.c_str(), status.c_str());

            char owner_info[256];
            sprintf_s(owner_info, sizeof(owner_info), "%s %s %s",
                reg_expiration.c_str(),
                owner_name.c_str(),
                owner_birthdate.c_str()
            );

            char owner_address[256];
            sprintf_s(owner_address, sizeof(owner_address), "%s %s",
                owner_address_1.c_str(),
                owner_address_2.c_str()
            );

            puttext_info(ainfo, plate_num.c_str(), aligned_status, owner_info, owner_address, vehicle_info,
                15, 25 + alert_count * 45);

            alert_count++;
        } else {
            cv::putText(info, plate_num.c_str(),
                cv::Point(15, 25 + vehicle_count * 45),
                FONT_HERSHEY_DUPLEX, 0.8,
                Scalar(102, 0, 0), 1, LINE_AA, false
            );
            cv::putText(info, vehicle_info,
                cv::Point(145, 20 + vehicle_count * 45),
                FONT_HERSHEY_DUPLEX, 0.4,
                Scalar(102, 0, 102), 0, LINE_AA, false
            );

            vehicle_count++;
        } 
    }

    if (pimag.empty())
        printf("plate image empty \n");
    else {
        cv::resize(pimag, pimag, Size(VI_WIDTH, 100));
        if (vehicle_count) {
            info.copyTo(vtext);
            pimag.copyTo(vimg);
        }
        if (alert_count) {
            ainfo.copyTo(atext);
            pimag.copyTo(aimg);

            cv::putText(aimg, "A L E R T",
                cv::Point(200, 20),
                FONT_HERSHEY_DUPLEX, 0.7,
                Scalar(0, 0, 255), 1, LINE_AA, false
            );
        }
    }
}

void UIManager::RefreshUI(void)
{
    Mat info(vtext.size(), CV_8UC3, Scalar(255, 255, 255));
    Mat img(vimg.size(), CV_8UC3, Scalar(255, 255, 255));
    Mat ainfo(atext.size(), CV_8UC3, Scalar(255, 255, 255));
    Mat aimage(aimg.size(), CV_8UC3, Scalar(255, 255, 255));

    info.copyTo(vtext);
    img.copyTo(vimg);
    ainfo.copyTo(atext);
    aimage.copyTo(aimg);

    UpdateVideo();
}

void UIManager::UpdateVideo(void)
{
    Mat ui, vmerge, amerge;

    vconcat(vimg, vtext, vmerge);
    vconcat(aimg, atext, amerge);
    vconcat(vmerge, amerge, vinfo);
    hconcat(video, vinfo, ui);

    imshow("Laptop Application", ui);
}

