/*
 * RequestUploadTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestUploadTask.h"

RequestUploadTask::RequestUploadTask() {
	// TODO Auto-generated constructor stub
	mUrl = UPLOAD_SERVER_FILE;
	mpCallback = NULL;
	mbDir = false;
	mfilePath = "";
}

RequestUploadTask::~RequestUploadTask() {
	// TODO Auto-generated destructor stub
}

void RequestUploadTask::SetCallback(IRequestUploadCallback* pCallback) {
	mpCallback = pCallback;
}

bool RequestUploadTask::HandleCallback(const string& url, bool requestRet, const char* buf, int size) {
	FileLog(
			SnifferLogFileName,
			"RequestUploadTask::HandleCallback( "
			"url : %s, "
			"requestRet : %s "
			")",
			url.c_str(),
			requestRet?"true":"false"
			);

	if (size < MAX_LOG_BUFFER) {
		FileLog(SnifferLogFileName, "RequestUploadTask::HandleCallback( buf( %d ) : %s )", size, buf);
	}

	bool bFlag = false;
	string filePath = "";

	if (requestRet) {
		// request success
		Json::Value dataJson;
		if( HandleResult(buf, size, &dataJson) ) {
			bFlag = true;
			filePath = dataJson[FILEPATH_LOWER].asString();
		}
	}

	if( mpCallback != NULL ) {
		mpCallback->OnUpload(bFlag, filePath, this);
	}

	return bFlag;
}

/**
 * @param deviceId			设备唯一标识
 */
void RequestUploadTask::SetParam(
		const string& deviceId,
		const string& filePath,
		bool bDir
		) {

	char temp[16];
	mHttpEntiy.Reset();
	mHttpEntiy.SetSaveCookie(false);

	if( deviceId.length() > 0 ) {
		mHttpEntiy.AddContent(DEVICE_ID_LOWER, deviceId);
	}

	if( filePath.length() > 0 ) {
		mfilePath = filePath;

		// 文件
		mHttpEntiy.AddFile(UPLOAD_FILE_LOWER, filePath, "image/jpg");

		// 目录名
		string dir;
		string::size_type pos = filePath.find_last_of("/");
		if( pos != string::npos ) {
			dir = filePath.substr(0, pos + 1);
			if( dir.length() > 0 ) {
				mHttpEntiy.AddContent(UPLOAD_DIR_LOWER, dir);
			}
		}
	}

	mbDir = bDir;

	FileLog(
			SnifferLogFileName,
			"RequestUploadTask::SetParam( "
			"filePath : %s "
			")",
			filePath.c_str()
			);
}

bool RequestUploadTask::IsDir() {
	return mbDir;
}

const string& RequestUploadTask::GetFilePath() {
	return mfilePath;
}
