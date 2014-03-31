/*
 * DrSnifferClient.h
 *
 *  Created on: 2014年3月9日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef DRSNIFFERCLIENT_H_
#define DRSNIFFERCLIENT_H_

#include "DrCommandDef.h"

#include "../common/DrTcpSocket.h"
#include "../common/DrThread.h"

class DrSnifferClient {
public:
	DrSnifferClient();
	virtual ~DrSnifferClient();

	// 连接服务器
	bool ConnectServer();
	// 接收服务器命令
	SCCMD RecvCommand();
	// 发送命令到服务器
	bool SendCommand(SSCMD sscmd);

	// 保持唤醒线程
	bool StartWakeThread();
	void StopWakeThread();

private:
	DrTcpSocket m_DrTcpSocket;
	string m_ServerAddress;
	int m_iServerPort;

	DrThread* m_pWakeThread;		// 唤醒设备线程
	DrCond m_DrCondWake;			// 唤醒设备线程信号
};

#endif /* DRSNIFFERCLIENT_H_ */
