// LaptopApplication.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <thread>

#include "LaptopModules.h"
#include "NetworkTCP.h"

using namespace client;

int main()
{
    std::string county = "us";

    client::MainController mc;
    client::UIManager ui;
    client::IOSourceManager io;
	client::VehicleInfoManager vehicleInfoMng;
	client::ALPRProcessor alpr(county, "", &vehicleInfoMng);
    client::CommunicationManager commMan;
    client::LoginManager loginMng;

    loginMng.linkCommMag(&commMan);
    vehicleInfoMng.linkCommMag(&commMan);
    commMan.networkConnect();

	if (loginMng.login())
		exit(1);

	vehicleInfoMng.VehicleInfoReceiveStart();

	mc.mode = ui.GetVideoMode();
	if (mc.mode == Mode::mNone)
		exit(0);

	if (mc.mode == Mode::mLive_Video)
	{
		mc.dev_id = ui.GetVideoDevice();
		if (mc.dev_id == -1) exit(0);
		mc.vres = ui.GetVideoResolution();
		if (mc.vres == VideoResolution::rNone) exit(0);
	}
	else
	{
		if (!ui.GetFileName(mc.mode, mc.inputfilename))
			exit(0);
	}
	
	if (mc.mode != Mode::mImage_File)
		io.videosavemode = ui.GetVideoSaveMode();

	alpr.setTopN(2);
	if (alpr.isLoaded() == false)
	{
		ui.PrintErrMsg("Error loading OpenALPR");
		return 1;
	}

	if (mc.mode != Mode::mImage_File)
	{
		if (!io.OpenInputVideo(mc.mode, mc.vres, mc.dev_id, mc.inputfilename)) {
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

	auto alpr_process = bind(&ALPRProcessor::process, alpr, placeholders::_1);
	thread io_handler(&IOSourceManager::process, io, move(mc.mode), move(alpr_process));

	io_handler.join();

	// When everything done, release the video capture and write object
	io.ClossAll();
	ui.destroyAll();
	loginMng.logout();

    return 0;
}