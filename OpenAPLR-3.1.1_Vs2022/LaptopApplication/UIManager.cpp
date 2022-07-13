
#include <iostream>
#include <windows.h>
#include <tchar.h>

#include "LaptopModules.h"
#include "DeviceEnumerator.h"

using namespace std;
using namespace client;

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
        std::cout << "Select Live Video, PlayBack File or Image File" << std::endl;
        std::cout << "1 - PlayBack File" << std::endl;
        std::cout << "2 - Image File" << std::endl;
        std::cout << "3 - Test Connection" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        else if (key.uChar.AsciiChar == '1') mode = Mode::mPlayback_Video;
        else if (key.uChar.AsciiChar == '2') mode = Mode::mImage_File;
        else if (key.uChar.AsciiChar == '3') mode = Mode::mTest_Connection;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (mode == Mode::mNone);
    return(mode);
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

    if (retval) std::cout << "Filename is" << filename << std::endl;
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