/*
 * SnifferClient.cpp
 *
 *  Created on: 2014年3月9日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "SnifferClient.h"

/*
 * 监听线程
 */
class SnifferClientRunnable : public KRunnable {
public:
	SnifferClientRunnable(SnifferClient *pSnifferClient) {
		mpSnifferClient = pSnifferClient;
	}
	virtual ~SnifferClientRunnable() {
		mpSnifferClient = NULL;
	}
protected:
	void onRun() {
		if( mpSnifferClient != NULL ) {
			mpSnifferClient->HandleSnifferClientRunnable();
		}
	}
private:
	SnifferClient *mpSnifferClient;
};

SnifferClient::SnifferClient() {
	// TODO Auto-generated constructor stub
	mpSnifferClientRunnable = new SnifferClientRunnable(this);

	mServerAddress = ServerAdess;
	miServerPort = ServerPort;

	mbRunning = false;
}

SnifferClient::~SnifferClient() {
	// TODO Auto-generated destructor stub
	if( mpSnifferClientRunnable ) {
		delete mpSnifferClientRunnable;
		mpSnifferClientRunnable = NULL;
	}
}

void SnifferClient::SetSnifferClientCallback(SnifferClientCallback *pSnifferClientCallback) {
	mpSnifferClientCallback = pSnifferClientCallback;
}

bool SnifferClient::Start() {
	bool bFlag = false;

	if( !mbRunning ) {
		mbRunning = true;

		if( -1 != mKThread.start(mpSnifferClientRunnable) ) {
			bFlag = true;
		} else {
			mbRunning = false;
		}
	}
	return bFlag;
}

void SnifferClient::Stop() {
	mbRunning = false;
	mKThread.stop();
}

void SnifferClient::HandleSnifferClientRunnable() {

	Json::Value root;
	Json::FastWriter writer;
	string param;

	SCMD scmd;
	int seq = 0;

	while( mbRunning ) {
		FileLog(
				SnifferLogFileName,
				"SnifferClient::HandleSnifferClientRunnable( 等待连接服务端 )"
				);

		if( ConnectServer() ) {
			char deviceId[128] = {'\0'};
			memset(deviceId, '\0', sizeof(deviceId));
			list<IpAddressNetworkInfo> infoList = IPAddress::GetNetworkInfoList();
			if( infoList.size() > 0 && infoList.begin() != infoList.end() ) {
				IpAddressNetworkInfo info = *(infoList.begin());
				GetMD5String(info.mac.c_str(), deviceId);
			}

			// 连接上服务端, 发送手机型号/手机号
			FileLog(
					SnifferLogFileName,
					"SnifferClient::HandleSnifferClientRunnable( "
					"已经连接上服务端, 发送手机信息, "
					"DeviceId : %s, "
					"厂商 : %s, "
					"型号 : %s, "
					"手机号 : %s "
					")",
					deviceId,
					GetPhoneBrand().c_str(),
					GetPhoneModel().c_str(),
					""
					);

			root.clear();
			root[DEVICE_ID] = deviceId;
			root[VERSION] = SnifferVersion;
			root[PHONE_INFO_BRAND] = GetPhoneBrand();
			root[PHONE_INFO_MODEL] = GetPhoneModel();
			root[PHONE_INFO_NUMBER] = "";
			root[IS_ROOT] = IsRoot();

			param = writer.write(root);

			bzero(&scmd, sizeof(SCMD));
			scmd.header.scmdt = SnifferTypeClientInfo;
			scmd.header.bNew = true;
			scmd.header.seq = seq++;
			scmd.header.len = MIN(param.length(), MAX_PARAM_LEN - 1);
			memcpy(scmd.param, param.c_str(), param.length());
			scmd.param[scmd.header.len] = '\0';
			SendCommand(scmd);

			FileLog(SnifferLogFileName, "SnifferClient::HandleSnifferClientRunnable( 等待接收命令  )");

			bool bCanRecv = true;
			while(bCanRecv) {
				// 开始接收命令
				SCMD scmd;
				bCanRecv = RecvCommand(scmd);
				if( bCanRecv ) {
					if( mpSnifferClientCallback != NULL ) {
						mpSnifferClientCallback->OnRecvCommand(this, scmd);
					}
				} else {
					// 与服务端连接已经断开
					FileLog(SnifferLogFileName, "SnifferClient::HandleSnifferClientRunnable( 与服务端连接已经断开 )");
					break;
				}
			}
		}
		sleep(10);
	}
}

bool SnifferClient::ConnectServer() {
	bool bFlag = false;

	int iRet = mTcpSocket.Connect(mServerAddress.c_str(), miServerPort, true);
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
			}

		} else {
			// 没有参数
			bFlag = true;
		}
	}

	FileLog(
			SnifferLogFileName,
			"SnifferClient::RecvCommand( "
			"scmd.header.scmdt : %d, "
			"scmd.header.seq : %d, "
			"scmd.header.bNew : %s, "
			"scmd.header.len : %d, "
			"scmd.param : %s, "
			"bFlag : %s "
			")",
			scmd.header.scmdt,
			scmd.header.seq,
			scmd.header.bNew?"true":"false",
			scmd.header.len,
			scmd.param,
			bFlag?"true":"false"
			);

	return bFlag;
}

/*
 * 发送命令到服务器
 */
bool SnifferClient::SendCommand(const SCMD &scmd) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCommand( "
			"scmd.header.scmdt : %d, "
			"scmd.header.seq : %d, "
			"scmd.header.bNew : %s, "
			"scmd.header.len : %d, "
			"scmd.param : %s "
			")",
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
