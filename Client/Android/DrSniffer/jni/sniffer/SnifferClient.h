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

class SnifferClient;
class SnifferClientCallback {
public:
	virtual ~SnifferClientCallback(){};
	virtual void OnConnected(SnifferClient* client) = 0;
	virtual void OnRecvCommand(SnifferClient* client, const SCMD &scmd) = 0;
};
class SnifferClientRunnable;
class SnifferClient {
public:
	SnifferClient();
	virtual ~SnifferClient();

	void SetSnifferClientCallback(SnifferClientCallback *pSnifferClientCallback);
	bool Start();
	void Stop();

	// 发送命令到服务器
	bool SendCommand(const SCMD &scmd);

	void HandleSnifferClientRunnable();

private:
	// 连接服务器
	bool ConnectServer();

	// 接收服务器命令
	bool RecvCommand(SCMD &scmd);

	SnifferClientCallback* mpSnifferClientCallback;
	bool mbRunning;

	KThread mKThread;
	SnifferClientRunnable* mpSnifferClientRunnable;

	KTcpSocket mTcpSocket;
	string mServerAddress;
	int miServerPort;

};

#endif /* SNIFFERCLIENT_H_ */
