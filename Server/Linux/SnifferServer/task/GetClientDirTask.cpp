/*
 * GetClientDirTask.cpp
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#include "GetClientDirTask.h"

GetClientDirTask::GetClientDirTask() {
	// TODO Auto-generated constructor stub
	mDir = "/";
}

GetClientDirTask::~GetClientDirTask() {
	// TODO Auto-generated destructor stub
}

void GetClientDirTask::GetSendCmd(SCMD* scmd) {
	scmd->header.scmdt = SnifferListDir;
	scmd->header.bNew = true;
	scmd->header.len = mDir.length();
	memcpy(scmd->param, mDir.c_str(), scmd->header.len);
}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
//		snprintf(buffer,
//				MAXLEN - 1,
//				"<html>\n<body>\n"
//				"%s"
//				"</body>\n</html>\n",
//				StringHandle::replace(scmd->param, "\n", "</br>\n").c_str()
//				);
//		len = strlen(buffer);

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
