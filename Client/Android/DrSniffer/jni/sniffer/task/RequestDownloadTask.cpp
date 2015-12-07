/*
 * RequestDownloadTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestDownloadTask.h"

RequestDownloadTask::RequestDownloadTask() {
	// TODO Auto-generated constructor stub
	mpCallback = NULL;
	mUrl = "";
	mFilePath = "";

	mbFinishOK = false;
	mErrCode = "";
}

RequestDownloadTask::~RequestDownloadTask() {
	// TODO Auto-generated destructor stub
}

void RequestDownloadTask::SetCallback(IRequestDownloadCallback* pCallback) {
	mpCallback = pCallback;
}

void RequestDownloadTask::SetParam(
		const string& url,
		const string& filePath
		) {

	char temp[16];

	mUrl = url;
	mFilePath = filePath;

	FileLog(
			SnifferLogFileName,
			"RequestDownloadTask::SetParam( "
			"url : %s, "
			"filePath : %s "
			")",
			url.c_str(),
			filePath.c_str()
			);
}

bool RequestDownloadTask::Start() {
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
			"RequestDownloadTask::Start( "
			"bFlag : %s "
			")",
			bFlag?"true":"false"
			);

	return bFlag;
}

void RequestDownloadTask::Stop() {
	mHttpDownloader.Stop();
	BaseTask::Stop();
}

bool RequestDownloadTask::IsFinishOK() {
	return mbFinishOK;
}

const char* RequestDownloadTask::GetErrCode() {
	return mErrCode.c_str();
}

void RequestDownloadTask::onSuccess(HttpDownloader* loader) {
	FileLog(
			SnifferLogFileName,
			"RequestDownloadTask::onSuccess( "
			"loader : %p "
			")",
			loader
			);

	if( mpCallback != NULL ) {
		mpCallback->OnDownload(true, mFilePath, this);
	}

	mbFinishOK = true;

	// Send message to main thread
	OnTaskFinish();
}

void RequestDownloadTask::onFail(HttpDownloader* loader) {
	FileLog(
			SnifferLogFileName,
			"RequestDownloadTask::onFail( "
			"loader : %p "
			")",
			loader
			);

	if( mpCallback != NULL ) {
		mpCallback->OnDownload(false, mFilePath, this);
	}

	mbFinishOK = false;

	// Send message to main thread
	OnTaskFinish();
}

void RequestDownloadTask::onUpdate(HttpDownloader* loader) {

}
