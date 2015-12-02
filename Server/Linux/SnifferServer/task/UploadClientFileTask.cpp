/*
 * UploadClientFileTask.cpp
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#include "UploadClientFileTask.h"

UploadClientFileTask::UploadClientFileTask() {
	// TODO Auto-generated constructor stub
	mClientId = 0;
	mFilePath = "/";
}

UploadClientFileTask::~UploadClientFileTask() {
	// TODO Auto-generated destructor stub
}

void UploadClientFileTask::GetSendCmd(SCMD* scmd) {
	scmd->header.scmdt = SnifferUploadFile;
	scmd->header.bNew = true;
	scmd->header.len = (int)mFilePath.length();
	memcpy(scmd->param, mFilePath.c_str(), scmd->header.len);
}

bool UploadClientFileTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
		memcpy(buffer, scmd->param, scmd->header.len);
		len = scmd->header.len;

		bFlag = true;
	}

	return bFlag;
}

void UploadClientFileTask::SetClientId(int clientId) {
	mClientId = clientId;
}

void UploadClientFileTask::SetFilePath(const char* filePath) {
	if( filePath != NULL ) {
		mFilePath = filePath;
	}
}
