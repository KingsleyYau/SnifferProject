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

void UploadClientFileTask::GetSendCmd(SCMD* scmd, int seq) {
	scmd->header.scmdt = SnifferUploadFile;
	scmd->header.bNew = true;
	scmd->header.seq = seq;
	scmd->header.len = (int)mFilePath.length();
	memcpy(scmd->param, mFilePath.c_str(), scmd->header.len);
}

bool UploadClientFileTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
		switch( mPtType ) {
		case HTML: {
			string result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
			result += "<pre>";

			char upload[512];

		    Json::Reader reader;
		    Json::Value rootRecv;
		    reader.parse(scmd->param, rootRecv);

		    if( rootRecv[COMMON_RET].isInt()
		    		&& rootRecv[COMMON_RET].asInt() != 0
		    		&& rootRecv[DOWN_SERVER_ADDRESS].isString()
		    		&& rootRecv[FILEPATH].isString()
		    		) {
		    	result += "已上传到: ";
				result += rootRecv[DOWN_SERVER_ADDRESS].asString();
				result += rootRecv[FILEPATH].asString();
				result += " ";

				result += "<a href=\"";
				result += rootRecv[DOWN_SERVER_ADDRESS].asString();
				result += DOWN_SERVER_FILE;
				result += "?";
				result += FILEPATH_LOWER;
				result += "=";
				result += rootRecv[FILEPATH].asString();
				result += "\">";
				result += "下载到本地";
				result += "</a>";
		    } else {
		    	result += "上传失败";
		    }

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
			memcpy(buffer, scmd->param, scmd->header.len);
			len = scmd->header.len;
		}break;
		}

		bFlag = true;
	}

	return bFlag;
}

void UploadClientFileTask::SetClientId(int clientId) {
	mClientId = clientId;
}

void UploadClientFileTask::SetFilePath(const string& filePath) {
	if( filePath.length() > 0 ) {
		mFilePath = filePath;
	} else {
		mFilePath = "/";
	}
}
