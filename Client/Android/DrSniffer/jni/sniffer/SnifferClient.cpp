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

/*
 * 连接服务器
 */
bool SnifferClient::ConnectServer() {
	bool bFlag = false;

	mServerAddress = ServerAdess;
	miServerPort = ServerPort;

	int iRet = mTcpSocket.Connect(mServerAddress, miServerPort, true);
	if(iRet > 0) {
		bFlag = true;
	}

	return bFlag;
}

/*
 * 接收服务器命令
 */
bool SnifferClient::RecvCommand(SCMD &scmd) {
	bool bFlag = false;
	bzero(&scmd, sizeof(SCMD));
	scmd.header.scmdt = SinfferTypeNone;

	// 接收命令头
	char HeadBuffer[sizeof(SCMDH) + 1] = {'\0'};
	int len = mTcpSocket.RecvData(HeadBuffer, sizeof(SCMDH));
	if(len == sizeof(SCMDH)) {
		// 接收头成功
		memcpy(&scmd.header, HeadBuffer, sizeof(SCMDH));

		if( scmd.header.len > 0 ) {
			len = mTcpSocket.RecvData(scmd.param, scmd.header.len);
			if( len == scmd.header.len ) {
				// 接收参数成功
				scmd.param[scmd.header.len] = '\0';
				bFlag = true;
			} else {
				// 接收参数失败
				scmd.header.scmdt = SinfferTypeNone;
			}
		} else {
			// 没有参数
			bFlag = true;
		}
	}

	FileLog(SnifferLogFileName, "收到服务器命令, [ "
			"scmd.header.scmdt : %d, "
			"scmd.header.seq : %d, "
			"scmd.header.bNew : %s, "
			"scmd.header.len : %d, "
			"scmd.param : %s "
			"]",
			scmd.header.scmdt,
			scmd.header.seq,
			scmd.header.bNew?"true":"false",
			scmd.header.len,
			scmd.param
			);

	return bFlag;
}

/*
 * 发送命令到服务器
 */
bool SnifferClient::SendCommand(const SCMD &scmd) {
	FileLog(SnifferLogFileName, "发送命令到服务器, [ "
			"scmd.header.scmdt : %d, "
			"scmd.header.seq : %d, "
			"scmd.header.bNew : %s, "
			"scmd.header.len : %d, "
			"scmd.param : %s "
			"]",
			scmd.header.scmdt,
			scmd.header.seq,
			scmd.header.bNew?"true":"false",
			scmd.header.len,
			scmd.param
			);

	bool bFlag = false;

	// 发送命令
	int iLen = sizeof(SCMDH) + scmd.header.len;
	int iSend = mTcpSocket.SendData((char *)&scmd, iLen);
	if( iSend == iLen ) {
		bFlag = true;
	}

	return bFlag;
}
