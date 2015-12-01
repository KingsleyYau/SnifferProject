/*
 * RequestUploadTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestUploadTask.h"

RequestUploadTask::RequestUploadTask() {
	// TODO Auto-generated constructor stub
	mUrl = "upload.cgi";
	mpCallback = NULL;
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
			"RequestUploadTask::HandleResult( "
			"url : %s,"
			"requestRet : %s "
			")",
			url.c_str(),
			requestRet?"true":"false"
			);

	if (size < MAX_LOG_BUFFER) {
		FileLog(SnifferLogFileName, "RequestUploadTask::HandleResult( buf( %d ) : %s )", size, buf);
	}

	bool bFlag = false;
	string filePath = "";

	if (requestRet) {
		// request success
		Json::Value dataJson;
		if( HandleResult(buf, size, &dataJson) ) {
			bFlag = true;
			filePath = dataJson["filePath"].asString();
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
		const string& filePath
		) {

	char temp[16];
	mHttpEntiy.Reset();
	mHttpEntiy.SetSaveCookie(true);

	if( deviceId.length() > 0 ) {
		mHttpEntiy.AddContent(DEVICE_ID, deviceId);
	}

	if( filePath.length() > 0 ) {
		mHttpEntiy.AddFile("upload_file", filePath, "image/jpg");
	}

	FileLog(
			SnifferLogFileName,
			"RequestUploadTask::SetParam( "
			"filePath : %s "
			")",
			filePath.c_str()
			);
}
