/*
 * SnifferClient.cpp
 *
 *  Created on: 2014年3月9日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "SnifferClient.h"

SnifferClient::SnifferClient() {
	// TODO Auto-generated constructor stub
	mServerAddress = ServerAdess;
	miServerPort = ServerPort;
}

SnifferClient::~SnifferClient() {
	// TODO Auto-generated destructor stub
}

// 保持唤醒线程
bool StartWakeThread() {
	bool bFlag = false;
	return bFlag;
}
void StopWakeThread() {

}
/*
 * 连接服务器
 */
bool SnifferClient::ConnectServer() {
	FileLog(SnifferLogFileName, "SnifferClient::ConnectServer( %s:%d )",
			mServerAddress.c_str(),
			miServerPort
			);
	bool bFlag = false;
	int iRet = mTcpSocket.Connect(mServerAddress, miServerPort, true);
	if(iRet > 0) {
		bFlag = true;
	}
	FileLog(SnifferLogFileName, "SnifferClient::ConnectServer( iRet : %d )", iRet);
	return bFlag;
}

/*
 * 接收服务器命令
 */
SCMD SnifferClient::RecvCommand() {
	FileLog(SnifferLogFileName, "SnifferClient::RecvCommand()");
	SCMD scmd;
	scmd.header.scmdt = SinfferTypeNone;

	// 接收命令头
	char HeadBuffer[sizeof(SCMDH) + 1] = {'\0'};
	int len = mTcpSocket.RecvData(HeadBuffer, sizeof(SCMDH));
	if(len == sizeof(SCMDH)) {
		// 接收头成功
		memcpy(&scmd.header, HeadBuffer, sizeof(SCMDH));

		len = mTcpSocket.RecvData(scmd.param, scmd.header.len);
		if( len == scmd.header.len ) {

		}
		scmd.param[len] = '\0';
	}
	return scmd;
}

/*
 * 发送命令到服务器
 */
bool SnifferClient::SendCommand(SCMD scmd) {
	FileLog(SnifferLogFileName, "SnifferClient::SendCommand()");
	bool bFlag = false;

	// 发送命令
	int iLen = sizeof(SCMDH) + scmd.header.len;
	int iSend = mTcpSocket.SendData((char *)&scmd, iLen);
	if( iSend == iLen ) {
		bFlag = true;
	}

	return bFlag;
}
