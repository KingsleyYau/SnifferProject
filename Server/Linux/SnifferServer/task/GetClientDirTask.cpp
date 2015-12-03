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
	mDir = "/";
	mPageIndex = 0;
	mPageSize = 20;
}

GetClientDirTask::~GetClientDirTask() {
	// TODO Auto-generated destructor stub
}

void GetClientDirTask::GetSendCmd(SCMD* scmd) {
	Json::FastWriter writer;
	Json::Value rootSend;
	string result = "";

	rootSend[COMMON_PAGE_INDEX] = mPageIndex;
	rootSend[COMMON_PAGE_SIZE] = mPageSize;

	if( mDir.length() == 0 ) {
		mDir = "/";
	}
	rootSend[DIRECTORY] = mDir;

	result = writer.write(rootSend);

	scmd->header.scmdt = SnifferListDir;
	scmd->header.bNew = true;
	scmd->header.len = (int)result.length();
	memcpy(scmd->param, result.c_str(), scmd->header.len);
}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
		switch( mPtType ) {
		case HTML: {
			string result = "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
			result += "<pre>";

			char temp[8];
			char clientId[8];
			sprintf(clientId, "%d", mClientId);

			char upload[512];
			string item = "";

		    Json::Reader reader;
		    Json::Value rootRecv;
		    reader.parse(scmd->param, rootRecv);

		    if( rootRecv[FILE_LIST].isArray() ) {
		    	Json::Value dirItem;
		    	for( int i = 0; i < (int)rootRecv[FILE_LIST].size(); i++ ) {
		    		dirItem = rootRecv[FILE_LIST].get(i, Json::Value::null);
		    		if( dirItem[D_NAME].isString() ) {

			    		if( dirItem[D_NAME].asString() != ".." ) {
			    			// 上传按钮
			    			result += "<a href=\"";
			    			result += UPLOAD_CLIENT_FILE;
			    			result += "?";
			    			result += CLIENT_ID;
			    			result += "=";
			    			result += clientId;
			    			result += "&";
			    			result += FILEPATH;
			    			result += "=";
			    			result += mDir + "/";
			    			result += dirItem[D_NAME].asString();
			    			result += "\">";
			    			result += "[上传]";
			    			result += "</a>";
			    			result += " ";

			    			// 删除
			    			result += "<a href=\"";
			    			result += SET_CLIENT_CMD;
			    			result += "?";
			    			result += CLIENT_ID;
			    			result += "=";
			    			result += clientId;
			    			result += "&";
			    			result += COMMAND;
			    			result += "=";
			    			result += "rm -rf ";
							result += mDir + "/";
							result += dirItem[D_NAME].asString();
							result += "&";
							result += "\">";
							result += "[删除]";
							result += "</a>";
							result += " ";

			    		} else {
			    			result += "[----------]";
							result += " ";
			    		}

		    			snprintf(
		    					upload, sizeof(upload) - 1,
		    					"%7s "
		    					"%8s bytes ",
		    					dirItem[D_MODE].asString().c_str(),
		    					dirItem[D_SIZE].asString().c_str()
		    					);
		    			result += upload;

			    		if( dirItem[D_TYPE] == DT_DIR || dirItem[D_TYPE] == DT_LNK ) {
			    			// 进入
			    			result += "<a href=\"";
			    			result += GET_CLIENT_DIR;
			    			result += "?";
			    			result += CLIENT_ID;
			    			result += "=";
			    			result += clientId;
			    			result += "&";
			    			result += DIRECTORY;
			    			result += "=";

							if( dirItem[D_NAME].asString() != ".." ) {
								result += mDir;
								result += dirItem[D_NAME].asString();
								result += "/";
							} else {
								string dir = mDir;
								if( dir.length() > 2 ) {
									string::size_type pos = mDir.find_last_of("/", dir.length() - 2);
									if( pos != string::npos ) {
										dir = mDir.substr(0, pos);
									}
								}
								if( dir.length() == 0 ) {
									dir = "/";
								}

								result += dir;
							}

							result += "&";
							result += COMMON_PAGE_SIZE;
							result += "=";
							sprintf(temp, "%d", mPageSize);
							result += temp;
							result += "\">";
							result += dirItem[D_NAME].asString().c_str();
							result += "</a>";

			    		} else {
			    			result += dirItem[D_NAME].asString().c_str();
			    		}

			    		result += "\n";
		    		}
		    	}
		    }
		    result += "\n";

	    	if( mPageIndex != 0 ) {
				result += "<a href=\"";
				result += GET_CLIENT_DIR;
				result += "?";
				result += CLIENT_ID;
				result += "=";
				result += clientId;
				result += "&";
				result += DIRECTORY;
				result += "=";
				result += mDir;
				result += "&";
				result += COMMON_PAGE_INDEX;
				result += "=";
				sprintf(temp, "%d", mPageIndex - 1);
				result += temp;
				result += "&";
				result += COMMON_PAGE_SIZE;
				result += "=";
				sprintf(temp, "%d", mPageSize);
				result += temp;
				result += "\">";
				result += "[上一页]";
				result += "</a>		";
	    	}

		    if( rootRecv[COMMON_TOTAL].isInt() ) {
		    	int total = rootRecv[COMMON_TOTAL].asInt();
			    if( (mPageIndex + 1) * mPageSize < total ) {
					result += "<a href=\"";
					result += GET_CLIENT_DIR;
					result += "?";
					result += CLIENT_ID;
					result += "=";
					result += clientId;
					result += "&";
					result += DIRECTORY;
					result += "=";
					result += mDir;
					result += "&";
					result += COMMON_PAGE_INDEX;
					result += "=";
					sprintf(temp, "%d", mPageIndex + 1);
					result += temp;
					result += "&";
					result += COMMON_PAGE_SIZE;
					result += "=";
					sprintf(temp, "%d", mPageSize);
					result += temp;
					result += "\">";
					result += "[下一页]";
					result += "</a>\n";
			    }
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

void GetClientDirTask::SetClientId(int clientId) {
	mClientId = clientId;
}

void GetClientDirTask::SetDir(const char* dir) {
	if( dir != NULL ) {
		mDir = dir;
	}
}

void GetClientDirTask::SetPageIndex(int pageIndex) {
	mPageIndex = pageIndex;
}

void GetClientDirTask::SetPageSize(int pageSize) {
	mPageSize = pageSize;
}
