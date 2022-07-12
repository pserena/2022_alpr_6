// LaptopApplication.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "LaptopModules.h"

using namespace client;

int main()
{
    std::string county = "us";

    client::MainController mc{};
    client::UIManager ui{};
    client::IOSourceManager io{};
    client::ALPRProcessor alpr(county, "");

    while (1) {
        mc.mode = ui.GetVideoMode();
        if (mc.mode == Mode::mNone) exit(0);

        if (mc.mode == Mode::mTest_Connection) {
            ui.destroyAll();
            return 0;
        }

        if (mc.mode == Mode::mLogin) {
            continue;
        }
        else if (mc.mode == Mode::mLogout) {
            continue;
        }

        if (!ui.GetFileName(mc.mode, mc.inputfilename))
            exit(0);

        if (mc.mode != Mode::mImage_File)
        {
            io.videosavemode = ui.GetVideoSaveMode();
        }

        alpr.setTopN(2);
        if (alpr.isLoaded() == false)
        {
            ui.PrintErrMsg("Error loading OpenALPR");
            return 1;
        }

        if (mc.mode == Mode::mPlayback_Video)
        {
            if (!io.OpenInputVideo(mc.inputfilename)) {
                ui.PrintErrMsg("Error opening video file");
                return -1;
            }

            if (io.videosavemode != VideoSaveMode::vNoSave)
            {
                if (!io.OpenOutputVideo()) {
                    ui.PrintErrMsg("Could not open the output video for write");
                    return -1;
                }
            }
        }
    }
}