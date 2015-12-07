/*
 * RequestDownloadTask.h
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#ifndef RequestDownloadTask_H_
#define RequestDownloadTask_H_

#include "BaseTask.h"
#include <httpclient/HttpDownloader.h>

class RequestDownloadTask;

class IRequestDownloadCallback {
public:
	virtual ~IRequestDownloadCallback(){};
	virtual void OnDownload(bool success, const string& filePath, RequestDownloadTask* task) = 0;
};

class RequestDownloadTask : public BaseTask, public IHttpDownloaderCallback {
public:
	RequestDownloadTask();
	virtual ~RequestDownloadTask();

	void SetCallback(IRequestDownloadCallback* pCallback);

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
	IRequestDownloadCallback* mpCallback;

	HttpDownloader mHttpDownloader;

	string mUrl;
	string mFilePath;

	bool mbFinishOK;
	string mErrCode;
};

#endif /* REQUESTDOWNLOADTASK_H_ */
