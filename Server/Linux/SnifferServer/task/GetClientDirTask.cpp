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

void GetClientDirTask::GetSendCmd(SCMD* scmd, int seq) {
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
	scmd->header.seq = seq;
	scmd->header.len = (int)result.length();
	memcpy(scmd->param, result.c_str(), scmd->header.len);
}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFlag = false;

	if( buffer != NULL ) {
		switch( mPtType ) {
		case HTML: {
			string result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
			result += "<pre>";

			char temp[8];
			char clientId[8];
			sprintf(clientId, "%d", mClientId);

			result += "<form action=\"";
			result += DOWNLOAD_CLIENT_FILE;
			result += "\" method=\"GET\">";
			result += "URL:<input type=\"text\" name=\"";
			result += URL;
			result += "\"/>  ";
			result += "文件名:<input type=\"text\" name=\"";
			result += FILENAME;
			result += "\"/> ";
			result += "<input type=\"hidden\" name=\"";
			result += CLIENT_ID;
			result += "\" value=\"";
			result += clientId;
			result += "\" />";
			result += "<input type=\"hidden\" name=\"";
			result += FILEPATH;
			result += "\" value=\"";
			result += mDir;
			result += "\" />";
			result += "<input type=\"submit\" value=\"下载到此目录\"/>";
			result += "</form>";
			result += "\n";

			result += "当前路径: ";
			result += mDir;
			result += "\n";

			char line[1024];
			string href = "";
			Arithmetic ari;

		    Json::Reader reader;
		    Json::Value rootRecv;
		    reader.parse(scmd->param, rootRecv);

		    if( rootRecv[FILE_LIST].isArray() ) {

				result += "文件总数: ";
				sprintf(temp, "%d", rootRecv[COMMON_TOTAL].asInt());
				result += temp;
				result += "\n\n";

		    	Json::Value dirItem;
		    	for( int i = 0; i < (int)rootRecv[FILE_LIST].size(); i++ ) {
		    		dirItem = rootRecv[FILE_LIST].get(i, Json::Value::null);
		    		if( dirItem[D_NAME].isString() ) {
			    		if( dirItem[D_NAME].asString() != ".." ) {
			    			// 上传按钮
			    			result += "<a href=\"";

			    			href = UPLOAD_CLIENT_FILE;
			    			href += "?";
			    			href += CLIENT_ID;
			    			href += "=";
			    			href += clientId;
			    			href += "&";
			    			href += FILEPATH;
			    			href += "=";
			    			href += mDir;
			    			href += dirItem[D_NAME].asString();
			    			ari.encode_url(href.c_str(), href.length(), line);
			    			result += line;

			    			result += "\">";
			    			result += "[上传]";
			    			result += "</a>";
			    			result += " ";

			    			// 删除
			    			result += "<a href=\"";

			    			href = SET_CLIENT_CMD;
			    			href += "?";
			    			href += CLIENT_ID;
			    			href += "=";
			    			href += clientId;
			    			href += "&";
			    			href += COMMAND;
			    			href += "=";
			    			href += "rm -r \"";
			    			href += mDir;
			    			href += dirItem[D_NAME].asString();
			    			href += "\"";
			    			ari.encode_url(href.c_str(), href.length(), line);
			    			result += line;

							result += "\">";
							result += "[删除]";
							result += "</a>";
							result += " ";

			    		} else {
			    			result += "[----------]";
							result += " ";
			    		}

		    			snprintf(
		    					line, sizeof(line) - 1,
		    					"%7s "
		    					"%10s bytes ",
		    					dirItem[D_MODE].asString().c_str(),
		    					dirItem[D_SIZE].asString().c_str()
		    					);
		    			result += line;

			    		if( dirItem[D_TYPE] == DT_DIR || dirItem[D_TYPE] == DT_LNK ) {
			    			// 进入
			    			result += "<a href=\"";

			    			href = GET_CLIENT_DIR;
			    			href += "?";
							href += CLIENT_ID;
							href += "=";
							href += clientId;
							href += "&";
							href += DIRECTORY;
							href += "=";
			    			string dir = mDir;
							if( dirItem[D_NAME].asString() == ".." ) {
								if( dir.length() > 2 ) {
									string::size_type pos = mDir.find_last_of("/", dir.length() - 2);
									if( pos != string::npos ) {
										dir = mDir.substr(0, pos + 1);
									}
								}
								href += dir;

							} else {
								href += dir;
								href += dirItem[D_NAME].asString();
								href += "/";
							}
							href += "&";
							href += COMMON_PAGE_SIZE;
							href += "=";
							sprintf(temp, "%d", mPageSize);
							href += temp;
							ari.encode_url(href.c_str(), href.length(), line);
							result += line;

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

				href = GET_CLIENT_DIR;
				href += "?";
				href += CLIENT_ID;
				href += "=";
				href += clientId;
				href += "&";
				href += DIRECTORY;
				href += "=";
				href += mDir;
				href += "&";
				href += COMMON_PAGE_INDEX;
				href += "=";
				sprintf(temp, "%d", mPageIndex - 1);
				href += temp;
				href += "&";
				href += COMMON_PAGE_SIZE;
				href += "=";
				sprintf(temp, "%d", mPageSize);
				href += temp;
    			ari.encode_url(href.c_str(), href.length(), line);
    			result += line;

				result += "\">";
				result += "[上一页]";
				result += "</a> ";
	    	}

		    if( rootRecv[COMMON_TOTAL].isInt() ) {
		    	int total = rootRecv[COMMON_TOTAL].asInt();
			    if( (mPageIndex + 1) * mPageSize < total ) {
					result += "<a href=\"";

					href = GET_CLIENT_DIR;
					href += "?";
					href += CLIENT_ID;
					href += "=";
					href += clientId;
					href += "&";
					href += DIRECTORY;
					href += "=";
					href += mDir;
					href += "&";
					href += COMMON_PAGE_INDEX;
					href += "=";
					sprintf(temp, "%d", mPageIndex + 1);
					href += temp;
					href += "&";
					href += COMMON_PAGE_SIZE;
					href += "=";
					sprintf(temp, "%d", mPageSize);
					href += temp;
	    			ari.encode_url(href.c_str(), href.length(), line);
	    			result += line;

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

void GetClientDirTask::SetDir(const string& dir) {
	mDir = dir;
	if( mDir.length() == 0 || mDir[mDir.length() -1] != '/' ) {
		mDir += "/";
	}
}

void GetClientDirTask::SetPageIndex(int pageIndex) {
	mPageIndex = pageIndex;
}

void GetClientDirTask::SetPageSize(int pageSize) {
	mPageSize = pageSize;
}
