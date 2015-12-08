/*
 * SnifferClient.h
 *
 *  Created on: 2014年3月9日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERCLIENT_H_
#define SNIFFERCLIENT_H_

#include <SnifferCommandDef.h>
#include <SinfferExecuteDef.h>

#include <common/KTcpSocket.h>
#include <common/KThread.h>
#include <common/IPAddress.h>
#include <common/command.h>
#include <common/md5.h>
#include <common/StringHandle.h>
#include <json/json/json.h>

typedef struct FileStruct {
	string name;
	string st_size;
	string mode;
	int type;
} FileStruct;

class SnifferClient;
class SnifferClientCallback {
public:
	virtual ~SnifferClientCallback(){};
	virtual void OnConnected(SnifferClient* client) = 0;
	virtual void OnDisConnected(SnifferClient* client) = 0;

	virtual void OnRecvCmdExcuteCommand(SnifferClient* client, int seq, const string& cmd) = 0;
	virtual void OnRecvCmdSnifferListDir(SnifferClient* client, int seq, const string& dir, int index, int size) = 0;
	virtual void OnRecvCmdSnifferUploadFile(SnifferClient* client, int seq, const string& filePath) = 0;
	virtual void OnRecvCmdSnifferDownloadFile(SnifferClient* client, int seq, const string& url, const string& filePath) = 0;

};
class SnifferClientRunnable;
class SnifferClient {
public:
	SnifferClient();
	virtual ~SnifferClient();

	void SetSnifferClientCallback(SnifferClientCallback *pSnifferClientCallback);
	bool Start();
	void Stop();

	/**
	 * 返回执行命令结果到服务器
	 */
	bool SendCmdExcuteCommand(bool bFlag, int seq, const string& result);

	/**
	 * 返回列目录结果到服务器
	 */
	bool SendCmdSnifferListDir(bool bFlag, int seq, const list<FileStruct>& itemList, int iTotal);

	/**
	 * 返回上传文件结果到服务器
	 */
	bool SendCmdSnifferUploadFile(bool bFlag, int seq, const string& website, const string& filePath);

	/**
	 * 返回下载文件结果到服务器
	 */
	bool SendCmdSnifferDownloadFile(bool bFlag, int seq, const string& filePath);

	// 处理线程
	void HandleSnifferClientRunnable();

private:
	/*
	 * 连接服务器
	 */
	bool ConnectServer();

	/*
	 * 发送命令到服务器
	 */
	bool SendCommand(const SCMD &scmd);

	/*
	 * 接收服务器命令
	 */
	bool RecvCommand(SCMD &scmd);

	/*
	 * 分发接收命令
	 */
	void OnRecvCommand(const SCMD &scmd);

	/*###################### 解析接收命令  ######################*/
	/**
	 *	解析执行命令
	 */
	void HandleRecvCmdExcuteCommand(const SCMD &scmd);
	/**
	 * 解析列目录
	 */
	void HandleRecvCmdSnifferListDir(const SCMD &scmd);
	/**
	 * 解析上传文件
	 */
	void HandleRecvCmdSnifferUploadFile(const SCMD &scmd);
	/**
	 * 解析下载文件
	 */
	void HandleRecvCmdSnifferDownloadFile(const SCMD &scmd);
	/*###################### 解析接收命令  ######################*/

	SnifferClientCallback* mpSnifferClientCallback;
	bool mbRunning;

	KThread mKThread;
	SnifferClientRunnable* mpSnifferClientRunnable;

	KTcpSocket mTcpSocket;
	string mServerAddress;
	int miServerPort;

};

#endif /* SNIFFERCLIENT_H_ */
