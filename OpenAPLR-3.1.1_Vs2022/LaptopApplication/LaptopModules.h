#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <opencv2/highgui.hpp>
#include <functional>
#include <map>

#include "alpr.h"

using namespace std;
using namespace cv;
using namespace alpr;

namespace client
{
	enum class Mode { mNone, mPlayback_Video, mLive_Video, mImage_File };
	enum class VideoResolution { rNone, r640X480, r1280X720 };
	enum class VideoSaveMode { vNone, vNoSave, vSave, vSaveWithNoALPR };
	enum class ResponseMode { ReadingHeader, ReadingMsg };

	class MainController {
	public:
		Mode mode;
		int dev_id;
		VideoResolution vres;
		char inputfilename[MAX_PATH];

		MainController(void) {
			mode = Mode::mNone;
			dev_id = -1;
			vres = VideoResolution::rNone;
			memset(inputfilename, 0, MAX_PATH);
		}
		virtual ~MainController(void) { }
	private:
	};

	class UIManager {
	public:
		Mode GetVideoMode(void);
		int GetVideoDevice(void);
		VideoResolution GetVideoResolution(void);
		bool GetFileName(Mode mode, char filename[MAX_PATH]);
		VideoSaveMode GetVideoSaveMode(void);
		void PrintErrMsg(std::string msg);

		UIManager(void) {
		}
		virtual ~UIManager(void) {
		}
		void destroyAll(void) {
			destroyAllWindows();
		}
	private:
	};

	class IOSourceManager {

	public:
		VideoSaveMode videosavemode;

		IOSourceManager(void) {
			videosavemode = VideoSaveMode::vNone;
			frameno = 0;
			frame_width = 0;
			frame_height = 0;
			memset(inputfile, 0, MAX_PATH);
		}
		virtual ~IOSourceManager(void) {

		}
		bool OpenInputVideo(Mode mode, VideoResolution vres, int dev_id, char filename[MAX_PATH]);
		bool OpenOutputVideo(void);
		void process(Mode mode, function<void(Mat)> alpr_process);
		void SaveOutputVideo(Mat frame);
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

	class CommunicationManager {
	public:
		CommunicationManager();
		virtual ~CommunicationManager();

		int networkConnect(void);
		int networkConnectClose(void);
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
		int setRecognizedInfo(string rs, int puid, Mat pimg);
	private:
		CommunicationManager* commMan;
	};

	class ALPRProcessor : public Alpr {
	public:
		ALPRProcessor(std::string str1, std::string str2, VehicleInfoManager *vim) : Alpr(str1, str2) {
			plate_uid = 0;
			viManager = vim;
		}
		virtual ~ALPRProcessor() {}
		void process(Mat frame);
	private:
		int plate_uid;
		VehicleInfoManager* viManager;
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