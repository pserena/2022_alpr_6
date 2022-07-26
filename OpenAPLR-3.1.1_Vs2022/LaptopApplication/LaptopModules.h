#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <opencv2/highgui.hpp>
#include <functional>
#include <map>
#include <fstream>


#include "alpr.h"
#include "json.hpp"

using namespace std;
using namespace cv;
using namespace alpr;
using json = nlohmann::json;

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
		Mat video;
		Mat vinfo;

		// normal
		Mat vtext;
		Mat vimg;

		// alert
		Mat atext;
		Mat aimg;

		bool connection_lost;

		UIManager(void);
		virtual ~UIManager(void) {
		}
		Mode GetVideoMode(void);
		int GetVideoDevice(void);
		VideoResolution GetVideoResolution(void);
		bool GetFileName(Mode mode, char filename[MAX_PATH]);
		VideoSaveMode GetVideoSaveMode(void);
		void PrintErrMsg(std::string msg);
		void UpdateVinfo(string plate_number, int puid, Mat pimag, json jsonRetPlateInfo, int nError);
		void UpdateVideo(void);
		void RefreshUI(void);

		void destroyAll(void) {
			destroyAllWindows();
		}
	private:
	};

	class IOSourceManager {

	public:
		VideoSaveMode videosavemode;

		IOSourceManager(UIManager *uiManager) {
			ui = uiManager;
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
		UIManager* ui;
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
		CommunicationManager(const string& ipaddr);
		virtual ~CommunicationManager();

		int networkConnect(void);
		int networkConnectClose(void);
		int retryNetworkConnectSave(bool saveRetry);
		int retryNetworkConnect(void);
		int sendCommunicationData(unsigned char* data);
		int receiveAuthenticateData(char* data);
		int receiveCommunicationData(char* data);
		int authenticate(string strID, string strPw);
		int sendRecognizedInfo(string rs, int puid);

		bool saveRetry = false;
	private:
		void reconnectThreadStart(void);
		void timer_start(std::function<void(CommunicationManager*)> func, unsigned int interval);
		string ipaddr_;
	};

	class VehicleInfoManager {
	public:
		VehicleInfoManager(UIManager* uiManager);
		virtual ~VehicleInfoManager();

		int linkCommMag(CommunicationManager* comMan);
		int VehicleInfoReceiveStart(void);
		//int sendVehicleInfo(unsigned char* vehicleData);
		int receiveCommunicationData(void);
		int setRecognizedInfo(string rs, int puid, Mat pimg);
	private:
		CommunicationManager* commMan;
		UIManager* ui;
		void recevieThreadStart(void);
		void timer_start(std::function<void(VehicleInfoManager*)> func, unsigned int interval);
		ofstream log_output_;
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
		int inputLoginInfo(void);
		int checkLoginSuccess(void);
	};
}