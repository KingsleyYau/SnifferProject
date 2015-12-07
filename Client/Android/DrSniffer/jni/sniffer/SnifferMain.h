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
	void OnRecvCommand(SnifferClient* client, const SCMD &scmd);

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
	 * 处理列目录
	 */
	void HandleGetClientDir(const SCMD &scmd);
	string GetFileMode(const struct stat* statbuf);

	/**
	 * 处理上传文件
	 */
	void HandleUploadFile(const SCMD &scmd);

	/**
	 * 处理下载文件
	 */
	void HandleDownloadFile(const SCMD &scmd);

	Sniffer mSniffer;
	SnifferClient mSnifferClient;

	HttpRequestHostManager mHttpRequestHostManager;
	HttpRequestManager mHttpRequestManager;
};

#endif /* SNIFFERMAIN_H_ */
