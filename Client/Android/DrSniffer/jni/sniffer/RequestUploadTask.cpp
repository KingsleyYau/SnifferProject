/*
 * RequestUploadTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestUploadTask.h"

RequestUploadTask::RequestUploadTask() {
	// TODO Auto-generated constructor stub
	mUrl = "Upload";
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
 * @param email				电子邮箱
 * @param password			密码
 * @param deviceId			设备唯一标识
 * @param versioncode		客户端内部版本号
 * @param model				移动设备型号
 * @param manufacturer		制造厂商
 */
void RequestUploadTask::SetParam(
		const string& filePath
		) {

	char temp[16];
	mHttpEntiy.Reset();
	mHttpEntiy.SetSaveCookie(true);

	if( filePath.length() > 0 ) {
	}


	FileLog(
			SnifferLogFileName,
			"RequestUploadTask::SetParam( "
			"filePath : %s "
			")",
			filePath.c_str()
			);
}
