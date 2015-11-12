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
	bool RecvCommand(SCMD &scmd);

	// 发送命令到服务器
	bool SendCommand(const SCMD &scmd);

private:
	KTcpSocket mTcpSocket;
	string mServerAddress;
	int miServerPort;
};

#endif /* SNIFFERCLIENT_H_ */
