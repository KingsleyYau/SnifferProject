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
	scmd->header.len = result.length();
	memcpy(scmd->param, result.c_str(), scmd->header.len);
}

bool UpdateClientTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFalg = false;
	return bFalg;
}

void UpdateClientTask::SetUrl(const string& url) {
	mUrl = url;
}

void UpdateClientTask::SetVersion(const string& version) {
	mVersion = version;
}
