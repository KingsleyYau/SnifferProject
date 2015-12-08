/*
 * RequestUpdateTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestUpdateTask.h"

RequestUpdateTask::RequestUpdateTask() {
	// TODO Auto-generated constructor stub
	mpCallback = NULL;
	mUrl = "";
	mFilePath = "";

	mbFinishOK = false;
	mErrCode = "";
}

RequestUpdateTask::~RequestUpdateTask() {
	// TODO Auto-generated destructor stub
}

void RequestUpdateTask::SetCallback(IRequestUpdateCallback* pCallback) {
	mpCallback = pCallback;
}

void RequestUpdateTask::SetParam(
		const string& url,
		const string& filePath
		) {

	char temp[16];

	mUrl = url;
	mFilePath = filePath;

	FileLog(
			SnifferLogFileName,
			"RequestUpdateTask::SetParam( "
			"url : %s, "
			"filePath : %s "
			")",
			url.c_str(),
			mFilePath.c_str()
			);
}

bool RequestUpdateTask::Start() {
	bool bFlag = false;

	if( BaseTask::Start() ) {
		if( mHttpDownloader.StartDownload(mUrl, mFilePath, this) ) {
			bFlag = true;
		} else {
			OnTaskFinish();
		}
	}

	FileLog(
			SnifferLogFileName,
			"RequestUpdateTask::Start( "
			"bFlag : %s "
			")",
			bFlag?"true":"false"
			);

	return bFlag;
}

void RequestUpdateTask::Stop() {
	mHttpDownloader.Stop();
	BaseTask::Stop();
}

bool RequestUpdateTask::IsFinishOK() {
	return mbFinishOK;
}

const char* RequestUpdateTask::GetErrCode() {
	return mErrCode.c_str();
}

void RequestUpdateTask::onSuccess(HttpDownloader* loader) {
	FileLog(
			SnifferLogFileName,
			"RequestUpdateTask::onSuccess( "
			"loader : %p "
			")",
			loader
			);

	if( mpCallback != NULL ) {
		mpCallback->OnUpdateFinish(true, mFilePath, this);
	}

	mbFinishOK = true;

	// Send message to main thread
	OnTaskFinish();
}

void RequestUpdateTask::onFail(HttpDownloader* loader) {
	FileLog(
			SnifferLogFileName,
			"RequestUpdateTask::onFail( "
			"loader : %p "
			")",
			loader
			);

	if( mpCallback != NULL ) {
		mpCallback->OnUpdateFinish(false, mFilePath, this);
	}

	mbFinishOK = false;

	// Send message to main thread
	OnTaskFinish();
}

void RequestUpdateTask::onUpdate(HttpDownloader* loader) {

}
