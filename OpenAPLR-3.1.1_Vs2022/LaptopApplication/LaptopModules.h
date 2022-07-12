#pragma once

namespace client
{
	enum class Mode { mNone, mLogin, mLogout, mPlayback_Video, mImage_File, mTest_Connection };
	enum class VideoResolution { rNone, r640X480, r1280X720 };
	enum class VideoSaveMode { vNone, vNoSave, vSave, vSaveWithNoALPR };
	enum class ResponseMode { ReadingHeader, ReadingMsg };

	class MainController {
	public:
	private:
	};

	class UIManager {
	public:
	private:
	};

	class InputSourceManager {
	public:
	private:
	};

	class ALPRProcessor {
	public:
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