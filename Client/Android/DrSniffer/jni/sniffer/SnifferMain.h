/*
 * SnifferMain.h
 *
 *  Created on: 2015-11-14
 *      Author: Max
 */

#ifndef SNIFFERMAIN_H_
#define SNIFFERMAIN_H_

#include "Sniffer.h"
#include "SnifferClient.h"

#include <RequestUploadTask.h>
#include <RequestDownloadTask.h>

#include <httpclient/HttpRequestHostManager.h>
#include <httpclient/HttpRequestManager.h>

#include <common/command.h>
#include <json/json/json.h>

class SnifferMain : public SnifferClientCallback,
					public ITaskCallback,
					public IRequestUploadCallback,
					public IRequestDownloadCallback {
public:
	SnifferMain();
	virtual ~SnifferMain();

	/**
	 * Implement from SnifferClientCallback
	 */
	void OnConnected(SnifferClient* client);
	void OnDisConnected(SnifferClient* client);
	void OnRecvCmdExcuteCommand(SnifferClient* client, int seq, const string& cmd);
	void OnRecvCmdSnifferListDir(SnifferClient* client, int seq, const string& dir, int index, int size);
	void OnRecvCmdSnifferUploadFile(SnifferClient* client, int seq, const string& filePath);
	void OnRecvCmdSnifferDownloadFile(SnifferClient* client, int seq, const string& url, const string& filePath);

	bool Run();

private:
	/**
	 * Implement from ITaskCallback
	 */
	void OnTaskFinish(ITask* pTask);

	/**
	 * Implement from IRequestUploadCallback
	 */
	void OnUpload(bool success, const string& filePath, RequestUploadTask* task);

	/**
	 * Implement from IRequestDownloadCallback
	 */
	void OnDownload(bool success, const string& filePath, RequestDownloadTask* task);

	/**
	 * 获取文件类型
	 */
	string GetFileMode(const struct stat* statbuf);

	Sniffer mSniffer;
	SnifferClient mSnifferClient;

	HttpRequestHostManager mHttpRequestHostManager;
	HttpRequestManager mHttpRequestManager;
};

#endif /* SNIFFERMAIN_H_ */
