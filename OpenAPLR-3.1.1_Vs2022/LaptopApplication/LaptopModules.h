#pragma once

#include <windows.h>
#include <opencv2/highgui.hpp>

#include "alpr.h"

using namespace cv;
using namespace alpr;

namespace client
{
	enum class Mode { mNone, mLogin, mLogout, mPlayback_Video, mImage_File, mTest_Connection };
	enum class VideoSaveMode { vNone, vNoSave, vSave, vSaveWithNoALPR };
	enum class ResponseMode { ReadingHeader, ReadingMsg };

	class MainController {
	public:
		Mode mode;
		char inputfilename[MAX_PATH];
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
		int frame_width;
		int frame_height;
		VideoCapture cap;
		VideoSaveMode videosavemode;
		VideoWriter outv;

		bool OpenInputVideo(char filename[MAX_PATH]) {
			cap.open(filename);
			if (cap.isOpened()) {
				// Default resolutions of the frame are obtained.The default resolutions are system dependent.
				frame_width = GetInputVideoProp(cv::CAP_PROP_FRAME_WIDTH);
				frame_height = GetInputVideoProp(cv::CAP_PROP_FRAME_HEIGHT);
				printf("Frame width= %d height=%d\n", frame_width, frame_height);
			}
			return cap.isOpened();
		}
		bool OpenOutputVideo(void) {
			outv.open("output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 25, Size(frame_width, frame_height), true);
			return outv.isOpened();
		}
	
	private:
		int GetInputVideoProp(int id) {
			return (int)cap.get(id);
		}
	};

	class ALPRProcessor : public Alpr {
	public:
		ALPRProcessor(std::string str1, std::string str2) : Alpr(str1, str2) {}
	private:
	};

	class VehicleInfoManager {
	public:
	private:
	};

	class LoginManager {
	public:
	private:
	};

	class CommunicationManager {
	public:
	private:
	};
}