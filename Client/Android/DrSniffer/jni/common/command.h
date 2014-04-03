/*
 * command.h
 *
 *  Created on: 2014年2月18日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */


#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>
using namespace std;

#include "DrLog.h"

#define PRODUCT_MODEL 	 "ro.product.model="	 		// 型号
#define PRODUCT_BRAND	 "ro.product.brand="	 		// 厂商
#define BUILD_VERSION	 "ro.build.version.release="	// android版本

/*
 * 获取设备信息
 */
static inline string GetPhoneInfo(string param) {
	string findName = " ";
	string sCommand = "cat /system/build.prop 2>&1";
	string model = "";

	if(param.length() == 0) {
		return model;
	}

	FILE *ptr = popen(sCommand.c_str(), "r");
	if(ptr != NULL) {
		char buffer[2048] = {'\0'};

		while(fgets(buffer, 2048, ptr) != NULL) {
			string reslut = buffer;
			if(string::npos != reslut.find(param)) {
				model = reslut.substr(param.length(), reslut.length() - param.length() - 1);
//				DLog("jni.command::GetPhoneInfo()", "找到设备信息:%s", model.c_str());
			}
		}
		pclose(ptr);
		ptr = NULL;
	}

	return model;
}

/*
 * 获取设备型号
 */
static inline string GetPhoneModel() {
	string model = GetPhoneInfo(PRODUCT_MODEL);
	if(model.length() > 0) {
		DLog("jni.command::GetPhoneModel()", "找到手机型号:%s", model.c_str());
	}
	return model;
}

/*
 * 获取设备厂商
 */
static inline string GetPhoneBrand() {
	string model = GetPhoneInfo(PRODUCT_BRAND);
	if(model.length() > 0) {
		DLog("jni.command::GetPhoneBrand()", "找到手机厂商:%s", model.c_str());
	}
	return model;
}

/*
 * 获取设备 android version
 */
static inline string GetPhoneBuildVersion() {
	string model = GetPhoneInfo(BUILD_VERSION);
	if(model.length() > 0) {
		DLog("jni.command::GetPhoneBuildVersion()", "找到手机Android版本:%s", model.c_str());
	}
	return model;
}

/*
 * 获取进程名的Pid
 */
static inline int GetProcessPid(string name) {
	DLog("jni.command::GetProcessPid", "正在获取进程(%s)Pid...", name.c_str());
	int iPid = -1;

	string findName = " ";
	findName += name;
	string sCommand = "ps 2>&1";
	string reslut = "";

	FILE *ptr = popen(sCommand.c_str(), "r");
	if(ptr != NULL) {
		char buffer[2048] = {'\0'};

		// 获取第一行字段
		int iColumn = -1, index = 0;
		if(fgets(buffer, 2048, ptr) != NULL) {
			char *p = strtok(buffer, " ");
			while(p != NULL) {
				reslut = p;
				if(reslut == "PID") {
					iColumn = index;
					break;
				}
				index++;
				p = strtok(NULL, " ");
			}
		}

		// 获取进程pid
		if(iColumn != -1) {
			while(fgets(buffer, 2048, ptr) != NULL) {
				string reslut = buffer;
				if(string::npos != reslut.find(findName.c_str())) {
					char *p = strtok(buffer, " ");
					for(int i = 0; p != NULL; i++) {
						if(i == iColumn) {
							// 找到进程pid
							iPid = atoi(p);
							DLog("jni.command::GetProcessPid", "找到进程Pid:%s", p);
							break;
						}
						p = strtok(NULL, " ");
					}
				}
			}
		}

		pclose(ptr);
		ptr = NULL;
	}

	return iPid;
}

/*
 * 运行命令
 */
static inline void SystemComandExecute(string command) {
	string sCommand = command;
	sCommand += " &>/dev/null";
	system(sCommand.c_str());
	DLog("jni.command::SystemComandExecute", "command:%s", sCommand.c_str());
}

/*
 * 运行带返回命令
 */
static inline string SystemComandExecuteWithResult(string command) {
	string result = "";
	string sCommand = command;
	sCommand += " 2>&1";

	FILE *ptr = popen(sCommand.c_str(), "r");
	if(ptr != NULL) {
		char buffer[2048] = {'\0'};
		while(fgets(buffer, 2048, ptr) != NULL) {
			result += buffer;
		}
		pclose(ptr);
		ptr = NULL;
	}
	DLog("jni.command::SystemComandExecuteWithResult", "command:%s \ncommand result:%s", sCommand.c_str(), result.c_str());
	return result;
}

/*
 * 以Root权限运行命令
 */
static inline void SystemComandExecuteWithRoot(string command) {
	string sCommand;
	sCommand = "echo \"";//"su -c \"";
	sCommand += command;
	sCommand += "\"|su";
	SystemComandExecute(sCommand);
}

/*
 * 以Root权限运行带返回命令
 */
static inline string SystemComandExecuteWithRootWithResult(string command) {
	string sCommand = command;
	sCommand = "echo \"";//"su -c \"";
	sCommand += command;
	sCommand += "\"|su";
	return SystemComandExecuteWithResult(sCommand);
}

/*
 * 重新挂载/system为可读写模式
 */
static inline bool MountSystem() {
	bool bFlag = false;

	char pBuffer[2048] = {'\0'};
	string result = "";

	// 获取原/system挂载
	FILE *ptr = popen("mount", "r");
	if(ptr != NULL) {

		while(fgets(pBuffer, 2048, ptr) != NULL) {
			result = pBuffer;

			if(string::npos != result.find("/system")) {
				// 找到/system挂载路径
				char* dev = strtok(pBuffer, " ");
				result = dev;
				DLog("jni.command::MountSystem", "找到/system挂载路径:%s", result.c_str());
				break;
			}
		}
		pclose(ptr);
		ptr = NULL;
	}

	// 更改挂载为rw
	sprintf(pBuffer, "mount -o remount rw,%s /system", result.c_str());
	result = SystemComandExecuteWithRootWithResult(pBuffer);
	if(result.length() == 0) {
		DLog("jni.command::MountSystem", "挂载/system为可读写成功!");
		bFlag = true;
	}
	else {
		ELog("jni.command::MountSystem", "挂载/system为可读写失败!");
	}

	return bFlag;
}

/*
 * 拷贝文件到指定目录下
 * destDirPath:	目标目录
 */
static inline bool RootNonExecutableFile(string sourceFilePath, string destDirPath, string destFileName = "") {
	bool bFlag = false;

	char pBuffer[2048] = {'\0'};
	string result = "";

	if(MountSystem()) {
		// 开始拷贝
		string fileName = sourceFilePath;

		if(destFileName.length() == 0) {
			// 没有指定文件名,用原来的名字
			string::size_type pos = string::npos;
			pos = sourceFilePath.find_last_of('/');
			if(string::npos != pos) {
				pos ++;
				fileName = sourceFilePath.substr(pos, sourceFilePath.length() - pos);
			}
			fileName = destDirPath + fileName;
		}
		else {
			// 指定文件名
			fileName = destDirPath + destFileName;
		}

		sprintf(pBuffer, "cat %s > %s", sourceFilePath.c_str(), fileName.c_str());
		result = SystemComandExecuteWithRootWithResult(pBuffer);
		if(result.length() == 0) {
			// 拷贝成功
			DLog("jni.command::RootNonExecutableFile", "拷贝%s到%s成功!", sourceFilePath.c_str(), fileName.c_str());
			bFlag = true;
		}
		else {
			ELog("jni.command::RootNonExecutableFile", "拷贝%s到%s失败!", sourceFilePath.c_str(), fileName.c_str());
		}
	}

	return bFlag;
}

/*
 * 安装可执行文件到 目标目录
 * destDirPath:	目标目录
 */
static inline bool RootExecutableFile(string sourceFilePath, string destDirPath = "/system/bin/", string destFileName = "") {
	bool bFlag = false;

	char pBuffer[2048] = {'\0'};
	string result = "";

	// 开始拷贝
	string fileName = "";

	if(destFileName.length() == 0) {
		// 没有指定文件名,用原来的名字
		string::size_type pos = string::npos;
		pos = sourceFilePath.find_last_of('/');
		if(string::npos != pos) {
			pos ++;
			fileName = sourceFilePath.substr(pos, sourceFilePath.length() - pos);
		}
	}
	else {
		// 指定文件名
		fileName = destFileName;
	}

	// 如果在运行先关闭
	int iPid = GetProcessPid(fileName);
	if(iPid != -1) {
		DLog("jni.command::RootExecutableFile", "发现%s(PID:%d)正在运行, 先杀掉!", fileName.c_str(), iPid);
		sprintf(pBuffer, "kill -9 %d", iPid);
		SystemComandExecuteWithRoot(pBuffer);
	}

	if(RootNonExecutableFile(sourceFilePath, destDirPath, fileName)) {
		fileName = destDirPath + fileName;
		sprintf(pBuffer, "chmod 4755 %s", fileName.c_str());
		result = SystemComandExecuteWithRootWithResult(pBuffer);
		if(result.length() == 0) {
			// 更改权限成功
			DLog("jni.command::RootExecutableFile", "提升%s权限为4755成功!", fileName.c_str());
			bFlag = true;
		}
		else {
			ELog("jni.command::RootExecutableFile", "提升%s权限为4755失败!", fileName.c_str());
		}
	}

	return bFlag;
}
#endif /* COMMAND_H_ */
