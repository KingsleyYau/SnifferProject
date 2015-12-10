/*
 * RequestScreenCapUpdateTask.h
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#ifndef RequestScreenCapUpdateTask_H_
#define RequestScreenCapUpdateTask_H_

#include "BaseTask.h"

#include <RequestDefine.h>
#include <httpclient/HttpRequest.h>
#include <json/json/json.h>

#include <unistd.h>

#include <string>
using namespace std;

class RequestScreenCapUpdateTask;
class ScreenCapRunnable;
class IRequestScreenCapUpdateCallback {
public:
	virtual ~IRequestScreenCapUpdateCallback(){};
	virtual void OnUploadScreenCap(bool success, const string& url, RequestScreenCapUpdateTask* task) = 0;
	virtual bool OnGetScreenCap(string& filePath, RequestScreenCapUpdateTask* task) = 0;
};

class RequestScreenCapUpdateTask : public BaseTask, public IHttpRequestCallback {
public:
	RequestScreenCapUpdateTask();
	virtual ~RequestScreenCapUpdateTask();

	void SetCallback(IRequestScreenCapUpdateCallback* pCallback);

	/**
	 * @param url
	 */
	void SetParam(
			const string& deviceId,
			const string& server,
			const string& path
			);

	void HandleTaskRunnable();

	// Implement from BaseTask
	bool Start();
	void Stop();
	bool IsFinishOK();
	const char* GetErrCode();

	void onSuccess(long requestId, string url, const char* buf, int size);
	void onFail(long requestId, string url);
	void onReceiveBody(long requestId, string url, const char* buf, int size);

protected:
	IRequestScreenCapUpdateCallback* mpCallback;

	bool mbFinishOK;
	string mErrCode;

	string mDeviceId;
	string mServer;
	string mPath;
	bool bIsRunning;

	KThread mThread;
	KCond mCond;
	ScreenCapRunnable* mpTaskRunnable;
	HttpRequest mHttpRequest;

};

#endif /* REQUESTSCREENCAPUPDATETASK_H_ */
