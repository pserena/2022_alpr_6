
#include "LaptopModules.h"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace client;

bool _qpcInited = false;
double PCFreq = 0.0;
__int64 CounterStart = 0;
double _avgdur = 0;
double _fpsstart = 0;
double _avgfps = 0;
double _fps1sec = 0;

static void InitCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		std::cout << "QueryPerformanceFrequency failed!\n";
	}
	PCFreq = double(li.QuadPart) / 1000.0f;
	_qpcInited = true;
}

static double CLOCK()
{
	if (!_qpcInited) InitCounter();
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart) / PCFreq;
}

static double avgdur(double newdur)
{
	_avgdur = 0.98 * _avgdur + 0.02 * newdur;
	return _avgdur;
}

static double avgfps()
{
	if (CLOCK() - _fpsstart > 1000)
	{
		_fpsstart = CLOCK();
		_avgfps = 0.7 * _avgfps + 0.3 * _fps1sec;
		_fps1sec = 0;
	}
	_fps1sec++;
	return _avgfps;
}

bool IOSourceManager::OpenInputVideo(Mode mode, VideoResolution vres, int dev_id, char filename[MAX_PATH])
{
	if (mode == Mode::mPlayback_Video) {
		cap.open(filename);
		memcpy(inputfile, filename, MAX_PATH);
	}
	else if (mode == Mode::mLive_Video)
		cap.open(dev_id, cv::CAP_ANY); // 0 = autodetect default API
	if (cap.isOpened()) {
		if (vres == VideoResolution::r1280X720)
		{
			cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
			cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
		}
		else if (vres == VideoResolution::r640X480)
		{
			cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
			cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
		}
		// Default resolutions of the frame are obtained.The default resolutions are system dependent.
		frameno = 0;
		frame_width = GetInputVideoProp(cv::CAP_PROP_FRAME_WIDTH);
		frame_height = GetInputVideoProp(cv::CAP_PROP_FRAME_HEIGHT);
		
		printf("Frame width= %d height=%d\n", frame_width, frame_height);
	}
	return cap.isOpened();
}

bool IOSourceManager::OpenOutputVideo(void)
{
	outv.open("output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 25, Size(frame_width, frame_height), true);
	return outv.isOpened();
}

void IOSourceManager::process(Mode mode, function<void(Mat)> alpr_process)
{
	char text[1024] = "";

	while (1) {
		Mat frame;
		
		double start = CLOCK();

		// Capture frame-by-frame
		if (mode == Mode::mImage_File)
		{
			frame = imread(inputfile);
		}
		else cap >> frame;

		if (frame.empty())
			break;

		if (videosavemode != VideoSaveMode::vSaveWithNoALPR)
		{
			alpr_process(frame);

			cv::putText(frame, text,
				cv::Point(10, frame.rows - 10), //top-left position
				FONT_HERSHEY_COMPLEX_SMALL, 0.5,
				Scalar(0, 255, 0), 0, LINE_AA, false);
		}

		// Write the frame into the file 'output.avi'
		if (videosavemode != VideoSaveMode::vNoSave)
		{
			thread output_saver(&IOSourceManager::SaveOutputVideo, this, frame);
			output_saver.detach();
		}

		// TODO: push to queue and 25 fps handling
		// Display the resulting frame
		imshow("Frame", frame);
		// Press  ESC on keyboard to  exit
		char c = (char)waitKey(1);
		if (c == 27)
			break;

		double dur = CLOCK() - start;
		sprintf_s(text, "avg time per frame %f ms. fps %f. frameno = %d", avgdur(dur), avgfps(), frameno++);
	}
}

void IOSourceManager::SaveOutputVideo(Mat frame)
{
	outv.write(frame);
}

void IOSourceManager::ClossAll(void)
{
	if (cap.isOpened())
		cap.release();
	if (outv.isOpened())
		outv.release();
}