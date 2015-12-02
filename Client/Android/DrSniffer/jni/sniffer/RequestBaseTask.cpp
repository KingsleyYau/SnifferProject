/*
 * RequestBaseTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "RequestBaseTask.h"

RequestBaseTask::RequestBaseTask() {
	// TODO Auto-generated constructor stub
	mRequestId = -1;
	mUrl = "";
	mNoCache = false;
	mSiteType = WebSite;
	mbFinishOK = false;
	mErrCode = "";
	mpErrcodeHandler = NULL;
}

RequestBaseTask::~RequestBaseTask() {
	// TODO Auto-generated destructor stub
}

void RequestBaseTask::Init(HttpRequestManager *pHttpRequestManager) {
	mpHttpRequestManager = pHttpRequestManager;
}

void RequestBaseTask::SetErrcodeHandler(ErrcodeHandler* pErrcodeHandler) {
	mpErrcodeHandler = pErrcodeHandler;
}

bool RequestBaseTask::Start() {
	if( BaseTask::Start() ) {
		if( -1 != StartRequest() ) {
			return true;
		} else {
			OnTaskFinish();
		}
	}
	return false;
}

void RequestBaseTask::Stop() {
	StopRequest();
	return BaseTask::Stop();
}

bool RequestBaseTask::IsFinishOK() {
	return mbFinishOK;
}

const char* RequestBaseTask::GetErrCode() {
	return mErrCode.c_str();
}

unsigned long RequestBaseTask::StartRequest() {
	if( mpHttpRequestManager != NULL && mUrl.length() > 0 ) {
		mRequestId = mpHttpRequestManager->StartRequest(
				mUrl,
				mHttpEntiy,
				this,
				mSiteType,
				mNoCache
				);
	}
	return mRequestId;
}

bool RequestBaseTask::StopRequest() {
	bool bFlag = false;
	if( mpHttpRequestManager != NULL && mRequestId != -1 ) {
		bFlag = mpHttpRequestManager->StopRequest(mRequestId);
		mRequestId = 0;
	}
	return bFlag;
}

bool RequestBaseTask::HandleResult(const char* buf, int size, Json::Value *data, bool* bContinue) {
	bool bFlag = false;
	bool bIsParseSuccess = false;
	if( bContinue != NULL ) {
		*bContinue = true;
	}

	/* try to parse json */
	Json::Value root;
	Json::Reader reader;

	if( reader.parse(buf, root, false) ) {
		FileLog(SnifferLogFileName, "RequestBaseTask::HandleResult( parse Json finish )");
		if( root.isObject() ) {
			bIsParseSuccess = true;

			if( root[COMMON_RESULT].isInt() && root[COMMON_RESULT].asInt() == 1 ) {
				if( data != NULL ) {
					*data = root[COMMON_DATA];
				}

				bFlag = true;
			} else {
				bFlag = false;
			}
		}
	}

	FileLog(SnifferLogFileName, "RequestBaseTask::HandleResult( handle json result %s )", bFlag?"ok":"fail");

	return bFlag;
}

/* IHttpRequestManagerCallback */
void RequestBaseTask::onSuccess(long requestId, string url, const char* buf, int size) {
	// Handle in sub class
	mbFinishOK = HandleCallback(url, true, buf, size);

	// Send message to main thread
	OnTaskFinish();
}

void RequestBaseTask::onFail(long requestId, string url) {
	// Handle in sub class
	mbFinishOK = HandleCallback(url, false, NULL, 0);

	// Send message to main thread
	OnTaskFinish();
}

/* Other */
// 获取下载总数据量及已收数据字节数
void RequestBaseTask::GetRecvDataCount(int& total, int& recv) const
{
	if (NULL != mpHttpRequestManager) {
		const HttpRequest* request = mpHttpRequestManager->GetRequestById(mRequestId);
		if (NULL != request) {
			request->GetRecvDataCount(total, recv);
		}
	}
}

// 获取上传总数据量及已收数据字节数
void RequestBaseTask::GetSendDataCount(int& total, int& send) const
{
	if (NULL != mpHttpRequestManager) {
		const HttpRequest* request = mpHttpRequestManager->GetRequestById(mRequestId);
		if (NULL != request) {
			request->GetSendDataCount(total, send);
		}
	}
}

// 获取 Content-Type
string RequestBaseTask::GetContentType() const
{
	string contentType("");
	if (NULL != mpHttpRequestManager) {
		const HttpRequest* request = mpHttpRequestManager->GetRequestById(mRequestId);
		if (NULL != request) {
			contentType = request->GetContentType();
		}
	}
	return contentType;
}
