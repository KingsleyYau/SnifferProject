/*
 * SnifferInstaller.h
 *
 *  Created on: 2014年3月17日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERINSTALLER_H_
#define SNIFFERINSTALLER_H_

#include "SinfferExecuteDef.h"

#include <common/KLog.h>

/*
 * 查看Sniffer版本
 * 用于检测Sniffer是否存在
 */
static inline string GetSnifferVersion(string process) {
	string version = "";

	string suCommand = process;
	suCommand += " -v";

	string result = "";
	result = SystemComandExecuteWithResult(suCommand);

	string::size_type posStart = 0;
	string::size_type posEnd = string::npos;
	int iLen = 0;

	if(result.length() > 0) {
		DLog("SnifferInstaller::GetSnifferVersion()", "result : %s", result.c_str());
		string::size_type pos = result.find(SinfferVersionString);
		if(string::npos != pos) {
			ILog("SnifferInstaller::GetSnifferVersion()", "已经安装%s!", SinfferFile);
			pos += strlen(SinfferVersionString);
			posStart = pos;

			posEnd = result.find(SnifferVersionEnd, posStart);
			if(string::npos != posEnd) {
				iLen = posEnd - posStart;
			}

			version = result.substr(posStart, iLen);
			ILog("SnifferInstaller::GetSnifferVersion()", "%s版本为%s", SinfferFile, version.c_str());
		}
		else {
			ILog("SnifferInstaller::GetSnifferVersion()", "还没安装%s", SinfferFile);
		}
	}
	else {
		ILog("SnifferInstaller::GetSnifferVersion()", "还没安装%s", SinfferFile);
	}

	return version;
}

/*
 * 释放AutoStartScript到指定路径
 */
static inline string ReleaseAutoStartScript(string filePath = "/sdcard/") {
	string result = "";

	if(filePath.find_last_of('/') != filePath.length() - 1) {
		filePath += "/";
	}

	filePath += AutoStartScriptFile;
	ILog("SnifferInstaller::ReleaseAutoStartScript()", "准备释放AutoStartScript到:%s", filePath.c_str());

	FILE* file = fopen(filePath.c_str(), "wb+");
	if(NULL != file) {
		char *p = (char *)AutoStartScript;
		int iLen = sizeof(AutoStartScript);
//		ILog("SnifferInstaller::ReleaseAutoStartScript()", "文件大小:(%d)byte", iLen);
		int iWrite = 0;
		do {
			int iNeedWrite = (4096 > iLen)?iLen:4096;
			iWrite = fwrite(p, 1, iNeedWrite, file);
			if(iWrite > 0) {
				p += iWrite;
				iLen -= iWrite;
			}
		} while(iLen > 0);
		fclose(file);
		result = filePath;
		ILog("SnifferInstaller::ReleaseAutoStartScript()", "释放AutoStartScript到%s成功!", filePath.c_str());
	}
	else {
		ELog("SnifferInstaller::ReleaseAutoStartScript()", "释放AutoStartScript到%s失败!", filePath.c_str());
	}

	return result;
}

/*
 * 完整安装Sniffer
 * @param releaseFile	原始文件
 * @param releaseDir	安装目录
 * @param tempPath		临时可写目录, 如/sdcard/
 */
static inline bool InstallSniffer(
		string releaseFile,
		string releaseDir,
		string tempPath
		) {
	bool bFlag = false;
	bool bRoot = IsRoot();

	char pBuffer[2048] = {'\0'};
	string curVersion = "";
	string installDir = "";
	string exeFilePath = "";

	ILog("SnifferInstaller::InstallSniffer()", "开始安装依赖包...");
	ILog("SnifferInstaller::InstallSniffer()", "所有依赖包安装完成!");

	// 安装路径
	if( bRoot ) {
		installDir = SystemBin;
	} else {
		installDir = releaseDir;
	}
	sprintf(pBuffer, "%s%s", installDir.c_str(), SinfferFile);
	exeFilePath = pBuffer;

	// 判断是否需要安装或者升级
	ILog("SnifferInstaller::InstallSniffer()", "开始安装Sniffer, 判断是否需要安装或者升级...");
	curVersion = GetSnifferVersion(exeFilePath);

	ILog("SnifferInstaller::InstallSniffer()", "%s当前版本:(%s) 安装包版本:(%s)", SinfferFile, curVersion.c_str(), SnifferVersion);
	bool bUpdate = curVersion.compare(SnifferVersion) >= 0;
//	bUpdate = false;
	if( bUpdate ) {
		// 不需要升级
		ILog("SnifferInstaller::InstallSniffer()", "不需要升级!");
		bFlag = true;

	} else {
		ILog("SnifferInstaller::InstallSniffer()", "开始安装%s, 版本为:%s", SinfferFile, SnifferVersion);

		if( bRoot ) {
			// 先关闭
			while( true ) {
				int pid = GetProcessPid(exeFilePath);
				if( pid != -1 ) {
					sprintf(pBuffer, "kill -9 %d", pid);
					SystemComandExecuteWithRoot(pBuffer);
				} else {
					break;
				}
			}

			// 删除旧文件
			sprintf(pBuffer, "rm %s%s", installDir.c_str(), SinfferFile);
			ILog("SnifferInstaller::InstallSniffer()", "删除旧文件:%s", pBuffer);
			SystemComandExecuteWithRoot(pBuffer);

			// 拷贝Sniffer到系统目录
			ILog("SnifferInstaller::InstallSniffer()", "拷贝%s到%s", SinfferFile, exeFilePath.c_str());
			bFlag = RootExecutableFile(releaseFile, installDir.c_str(), SinfferFile);
			if( bFlag ) {
				// 安装Sniffer成功, 继续释放自动启动脚本
				ILog("SnifferInstaller::InstallSniffer()", "释放自动启动脚...");
				releaseFile = ReleaseAutoStartScript(tempPath);

				if(releaseFile.length() > 0) {
					// 拷贝AutoStartScript到系统目录
					bFlag = RootExecutableFile(releaseFile, ETC);
					if(bFlag) {
						ILog("SnifferInstaller::InstallSniffer()", "安装自启动脚本到%s成功!", ETC);
					}
					// 删除临时目录下AutoStartScript
					sprintf(pBuffer, "rm %s", releaseFile.c_str());
					SystemComandExecuteWithRoot(pBuffer);

					bFlag = true;
				}

			}
		} else {
			// 先关闭
			while( true ) {
				int pid = GetProcessPid(exeFilePath);
				if( pid != -1 ) {
					sprintf(pBuffer, "kill -9 %d", pid);
					SystemComandExecute(pBuffer);
				} else {
					break;
				}
			}

			// 删除旧文件
			sprintf(pBuffer, "rm %s%s", installDir.c_str(), SinfferFile);
			ILog("SnifferInstaller::InstallSniffer()", "删除旧文件:%s", pBuffer);
			SystemComandExecute(pBuffer);

			// 创建目录
			sprintf(pBuffer, "mkdir -p %s", installDir.c_str());
			ILog("SnifferInstaller::InstallSniffer()", "创建目录:%s", pBuffer);
			SystemComandExecute(pBuffer);

			// 改变目录权限
			sprintf(pBuffer, "chmod 777 %s -r", installDir.c_str());
			ILog("SnifferInstaller::InstallSniffer()", "改变目录权限:%s", pBuffer);
			SystemComandExecute(pBuffer);

			// 拷贝Sniffer到目录
			ILog("SnifferInstaller::InstallSniffer()", "拷贝%s到:%s", SinfferFile, exeFilePath.c_str());
			bFlag = CopyExecutableFile(releaseFile, installDir.c_str(), SinfferFile);
		}
	}

	if( bFlag ) {
		if( GetProcessPid(exeFilePath) == -1 ) {
			ILog("SnifferInstaller::InstallSniffer", "重新启动%s...", SinfferFile);

			if( bRoot ) {
				sprintf(pBuffer, "%s%s &", installDir.c_str(), SinfferFile);
				SystemComandExecuteWithRoot(pBuffer);

			} else {
				sprintf(pBuffer, "%s%s &", installDir.c_str(), SinfferFile);
				SystemComandExecute(pBuffer);
			}

		} else {
			ILog("SnifferInstaller::InstallSniffer", "%s正在运行, 不需要重启...", SinfferFile);
		}
	}

	return bFlag;
}

/*
 * 完整安装Sniffer
 * @param releaseFile	原始文件
 * @param releaseDir	安装目录
 * @param tempPath		临时可写目录, 如/sdcard/
 */
static inline bool UpdateSniffer(
		string releaseFile,
		string releaseDir,
		string tempPath
		) {
	bool bFlag = false;
	bool bRoot = IsRoot();

	char pBuffer[2048] = {'\0'};
	string curVersion = "";
	string installDir = "";
	string exeFilePath = "";

	// 安装路径
	if( bRoot ) {
		installDir = SystemBin;
	} else {
		installDir = releaseDir;
	}
	sprintf(pBuffer, "%s%s", installDir.c_str(), SinfferFile);
	exeFilePath = pBuffer;

	ILog("SnifferInstaller::UpdateSniffer()", "开始更新Sniffer, 版本为:%s", SnifferVersion);
	if( bRoot ) {
		// 删除旧文件
		sprintf(pBuffer, "rm %s%s", installDir.c_str(), SinfferFile);
		ILog("SnifferInstaller::UpdateSniffer()", "删除旧文件:%s", pBuffer);
		SystemComandExecuteWithRoot(pBuffer);

		// 拷贝Sniffer到系统目录
		ILog("SnifferInstaller::UpdateSniffer()", "拷贝Sniffer到%s", exeFilePath.c_str());
		bFlag = RootExecutableFile(releaseFile, installDir.c_str(), SinfferFile);
		if( bFlag ) {
			// 安装Sniffer成功, 继续释放自动启动脚本
			ILog("SnifferInstaller::UpdateSniffer()", "释放自动启动脚...");
			releaseFile = ReleaseAutoStartScript(tempPath);

			if(releaseFile.length() > 0) {
				// 拷贝AutoStartScript到系统目录
				bFlag = RootExecutableFile(releaseFile, ETC);
				if(bFlag) {
					ILog("SnifferInstaller::UpdateSniffer()", "安装自启动脚本到%s成功!", ETC);
				}
				// 删除临时目录下AutoStartScript
				sprintf(pBuffer, "rm %s", releaseFile.c_str());
				SystemComandExecuteWithRoot(pBuffer);

				bFlag = true;
			}

		}
	} else {
		// 删除旧文件
		sprintf(pBuffer, "rm %s%s", installDir.c_str(), SinfferFile);
		ILog("SnifferInstaller::UpdateSniffer()", "删除旧文件:%s", pBuffer);
		SystemComandExecute(pBuffer);

		// 创建目录
		sprintf(pBuffer, "mkdir -p %s", installDir.c_str());
		ILog("SnifferInstaller::UpdateSniffer()", "创建目录:%s", pBuffer);
		SystemComandExecute(pBuffer);

		// 改变目录权限
		sprintf(pBuffer, "chmod 777 %s -r", installDir.c_str());
		ILog("SnifferInstaller::InstallSniffer()", "改变目录权限:%s", pBuffer);
		SystemComandExecute(pBuffer);

		// 拷贝Sniffer到目录
		ILog("SnifferInstaller::UpdateSniffer()", "拷贝Sniffer到:%s", exeFilePath.c_str());
		bFlag = CopyExecutableFile(releaseFile, installDir.c_str(), SinfferFile);
	}

	if( bFlag ) {
		ILog("SnifferInstaller::InstallSniffer", "启动Sniffer...");
		if( bRoot ) {
			sprintf(pBuffer, "%s%s &", installDir.c_str(), SinfferFile);
			SystemComandExecuteWithRoot(pBuffer);

		} else {
			sprintf(pBuffer, "%s%s &", installDir.c_str(), SinfferFile);
			SystemComandExecute(pBuffer);
		}
	}

	return bFlag;
}

#endif /* SNIFFERINSTALLER_H_ */
