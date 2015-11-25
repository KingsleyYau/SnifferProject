/*
 * GetClientDirTask.cpp
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#include "GetClientDirTask.h"

GetClientDirTask::GetClientDirTask() {
	// TODO Auto-generated constructor stub
	mClientId = 0;
	mDir = "";
	mPageIndex = 0;
	mPageSize = 20;
}

GetClientDirTask::~GetClientDirTask() {
	// TODO Auto-generated destructor stub
}

void GetClientDirTask::GetSendCmd(SCMD* scmd) {
	Json::FastWriter writer;
	Json::Value rootSend;
	string result;

	rootSend[DIRECTORY] = mDir;
	rootSend[COMMON_PAGE_SIZE] = mPageSize;
	rootSend[COMMON_PAGE_INDEX] = mPageIndex;
	result = writer.write(rootSend);

	scmd->header.scmdt = SnifferListDir;
	scmd->header.bNew = true;
	scmd->header.len = result.length();
	memcpy(scmd->param, result.c_str(), scmd->header.len);
}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
		string result = "";
		result += "<html>\n<body>\n";

		char clientId[8];
		sprintf(clientId, "%d", mClientId);

	    Json::Reader reader;
	    Json::Value rootRecv;
	    reader.parse(scmd->param, rootRecv);
	    if( rootRecv[FILE_LIST].isArray() ) {
	    	Json::Value dirItem;
	    	for( int i = 0; i < rootRecv[FILE_LIST].size(); i++ ) {
	    		dirItem = rootRecv[FILE_LIST].get(i, Json::Value::null);
	    		if( dirItem[D_NAME].isString() ) {
		    		if( dirItem[D_TYPE] == DT_DIR ) {
						result += "<a href=\"";
						result += GET_CLIENT_DIR;
						result += "?";
						result += CLIENT_ID;
						result += "=";
						result += clientId;
						result += "&";
						result += DIRECTORY;
						result += "=";
						result += mDir + "/";
						result += dirItem[D_NAME].asString();
						result += "\">";
						result += dirItem[D_NAME].asString();
						result += "</a></br>\n";
		    		} else {
		    			result += dirItem[D_NAME].asString();
		    		}
	    		}
	    	}
	    }

	    result += "</body>\n</html>\n";

		snprintf(
				buffer,
				MAXLEN - 1,
				"%s",
				result.c_str()
				);
		len = strlen(buffer);

		bFlag = true;
	}

	return bFlag;
}

void GetClientDirTask::SetDir(const char* dir) {
	if( dir != NULL ) {
		mDir = dir;
	}
}

void GetClientDirTask::SetClientId(int clientId) {
	mClientId = clientId;
}

void GetClientDirTask::SetPageIndex(int pageIndex) {
	mPageIndex = pageIndex;
}

void GetClientDirTask::SetPageSize(int pageSize) {
	mPageSize = pageSize;
}
