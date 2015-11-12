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

#include <common/KTcpSocket.h>
#include <common/KThread.h>

class SnifferClient {
public:
	SnifferClient();
	virtual ~SnifferClient();

	// 连接服务器
	bool ConnectServer();
	// 接收服务器命令
	SCMD RecvCommand();
	// 发送命令到服务器
	bool SendCommand(SCMD scmd);

	// 保持唤醒线程
	bool StartWakeKThread();
	void StopWakeKThread();

private:
	KTcpSocket mTcpSocket;
	string mServerAddress;
	int miServerPort;

	KThread* mpWakeKThread;		// 唤醒设备线程
	KCond mCondWake;			// 唤醒设备线程信号
};

#endif /* SNIFFERCLIENT_H_ */
