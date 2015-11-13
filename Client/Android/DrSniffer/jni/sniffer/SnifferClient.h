/*
 * SnifferClient.h
 *
 *  Created on: 2014年3月9日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERCLIENT_H_
#define SNIFFERCLIENT_H_

#include "SnifferCommandDef.h"
#include "SinfferExecuteDef.h"
#include "RequestUploadTask.h"

#include <common/KTcpSocket.h>
#include <common/KThread.h>

#include <httpclient/HttpRequestHostManager.h>
#include <httpclient/HttpRequestManager.h>

class SnifferClient : public ITaskCallback {
public:
	SnifferClient();
	virtual ~SnifferClient();

	/**
	 * Implement from ITaskCallback
	 */
	void OnTaskFinish(ITask* pTask);

	// 连接服务器
	bool ConnectServer();

	// 接收服务器命令
	bool RecvCommand(SCMD &scmd);

	// 发送命令到服务器
	bool SendCommand(const SCMD &scmd);

	// 上传文件到服务器
	bool UploadFile(const string& filePath);

private:
	KTcpSocket mTcpSocket;
	string mServerAddress;
	int miServerPort;

	HttpRequestHostManager mHttpRequestHostManager;
	HttpRequestManager mHttpRequestManager;
};

#endif /* SNIFFERCLIENT_H_ */
