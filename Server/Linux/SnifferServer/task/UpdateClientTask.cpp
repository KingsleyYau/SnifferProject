/*
 * UpdateClientTask.cpp
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#include "UpdateClientTask.h"

UpdateClientTask::UpdateClientTask() {
	// TODO Auto-generated constructor stub
	mUrl = "";
	mVersion = "";
}

UpdateClientTask::~UpdateClientTask() {
	// TODO Auto-generated destructor stub
}

void UpdateClientTask::GetSendCmd(SCMD* scmd) {
	Json::FastWriter writer;
	Json::Value rootSend;
	string result = "";

	rootSend[CLIENT_UPDATE_URL] = mUrl;
	rootSend[CLIENT_UPDATE_VERSION] = mVersion;

	result = writer.write(rootSend);

	scmd->header.scmdt = SnifferCheckUpdate;
	scmd->header.bNew = true;
	scmd->header.len = mCommand.length();
	memcpy(scmd->param, mCommand.c_str(), scmd->header.len);
}

bool UpdateClientTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFalg = false;
	if( buffer != NULL ) {
		switch( mPtType ) {
		case HTML: {
			string result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
			result += "<pre>";

		    result += "</pre>";
		    result += "</body></html>";

			snprintf(
					buffer,
					MAXLEN - 1,
					"%s",
					result.c_str()
					);

			len = strlen(buffer);

		}break;
		case JSON:{
			Json::FastWriter writer;
			Json::Value rootSend;

			rootSend[COMMON_RET] = ret;
			string result = writer.write(rootSend);

			len = result.length();
			memcpy(buffer, result.c_str(), len);

		}break;
		}

		bFlag = true;
	}

	return bFalg;
}

void UpdateClientTask::SetUrl(const string& url) {
	mUrl = url;
}

void UpdateClientTask::SetVersion(const string& version) {
	mVersion = version;
}
