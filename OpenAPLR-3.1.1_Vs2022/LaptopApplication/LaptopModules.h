#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <opencv2/highgui.hpp>
#include <functional>

#include "alpr.h"

using namespace std;
using namespace cv;
using namespace alpr;
using namespace std;

namespace client
{
	enum class Mode { mNone, mLogin, mLogout, mPlayback_Video, mImage_File, mTest_Connection };
	enum class VideoSaveMode { vNone, vNoSave, vSave, vSaveWithNoALPR };
	enum class ResponseMode { ReadingHeader, ReadingMsg };

	using fp = void (*)(Mat);

	class MainController {
	public:
		Mode mode;
		char inputfilename[MAX_PATH];

		MainController() {
			mode = Mode::mNone;
			memset(inputfilename, 0, MAX_PATH);
		}
	private:
	};

	class UIManager {
	public:
		Mode GetVideoMode(void);
		bool GetFileName(Mode mode, char filename[MAX_PATH]);
		VideoSaveMode GetVideoSaveMode(void);
		void PrintErrMsg(std::string msg);

		void destroyAll(void) {
			destroyAllWindows();
		}
	private:
	};

	class IOSourceManager {

	public:
		VideoSaveMode videosavemode;

		bool OpenInputVideo(char filename[MAX_PATH]);
		bool OpenOutputVideo(void);
		void process(Mode mode, function<void(Mat*)> alpr_process);
		void ClossAll(void);
	
	private:
		int frameno;
		int frame_width;
		int frame_height;
		VideoCapture cap;
		VideoWriter outv;
		char inputfile[MAX_PATH];

		int GetInputVideoProp(int id) {
			return (int)cap.get(id);
		}
	};

	class ALPRProcessor : public Alpr {
	public:
		ALPRProcessor(std::string str1, std::string str2) : Alpr(str1, str2) {}
		void process(Mat *frame);
	private:
	};

	class CommunicationManager {
	public:
		CommunicationManager();
		virtual ~CommunicationManager();

		int networkConnect(void);
		int networkConnectColse(void);
		int retryNetworkConnect(void);
		int sendCommunicationData(unsigned char* data);
		int receiveCommunicationData(char* data);
		int authenticate(string strID, string strPw);
	private:
	};

	class VehicleInfoManager {
	public:
		VehicleInfoManager();
		virtual ~VehicleInfoManager();

		int linkCommMag(CommunicationManager* comMan);
		int sendVehicleInfo(unsigned char* vehicleData);
		int receiveCommunicationData(char* vehicleData);
	private:
		CommunicationManager* commMan;
	};

	class LoginManager {
	public:
		LoginManager();
		virtual ~LoginManager();

		int linkCommMag(CommunicationManager* comMan);
		int login(void);
		int logout(void);
	private:
		CommunicationManager* commMan;
	};

}