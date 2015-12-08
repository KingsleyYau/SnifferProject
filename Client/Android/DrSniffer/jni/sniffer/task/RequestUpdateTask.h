/*
 * RequestUpdateTask.h
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#ifndef RequestUpdateTask_H_
#define RequestUpdateTask_H_

#include "BaseTask.h"
#include <httpclient/HttpDownloader.h>

class RequestUpdateTask;

class IRequestUpdateCallback {
public:
	virtual ~IRequestUpdateCallback(){};
	virtual void OnUpdateFinish(bool success, const string& filePath, RequestUpdateTask* task) = 0;
};

class RequestUpdateTask : public BaseTask, public IHttpDownloaderCallback {
public:
	RequestUpdateTask();
	virtual ~RequestUpdateTask();

	void SetCallback(IRequestUpdateCallback* pCallback);

    /**
     * @param url					下载路径
     * @param filePath				文件路径
     */
	void SetParam(
			const string& url,
			const string& filePath
			);

	// Implement from BaseTask
	bool Start();
	void Stop();
	bool IsFinishOK();
	const char* GetErrCode();

	/**
	 * Implement from IHttpDownloaderCallback
	 */
	void onSuccess(HttpDownloader* loader);
	void onFail(HttpDownloader* loader);
	void onUpdate(HttpDownloader* loader);

protected:
	IRequestUpdateCallback* mpCallback;

	HttpDownloader mHttpDownloader;

	string mUrl;
	string mFilePath;

	bool mbFinishOK;
	string mErrCode;
};

#endif /* REQUESTUPDATETASK_H_ */
