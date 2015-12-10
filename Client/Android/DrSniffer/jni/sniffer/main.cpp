/*
 * SnifferMain.cpp
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "SnifferMain.h"

#include <crashhandler/CrashHandler.h>

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
	if( ( setgid(gid) < 0 ) || ( setuid(uid) < 0 ) ) {
		ELog("sniffer", "权限不够, 部分功能可能缺失!");
//		return 0;
	}

	if( !MountSystem() ) {
		ELog("sniffer", "重新挂载/system失败, 部分功能可能缺失!");
	}

	KLog::SetLogDirectory("/sdcard/sniffer/");
	FileLog(SnifferLogFileName, "sniffer启动, 版本:%s", SnifferVersion);

//	CrashHandler::GetInstance()->SetLogDirectory("/sdcard/sniffer/");
	CrashHandler::GetInstance()->SetCrashLogDirectory("/sdcard/sniffer/");

	SnifferMain sniffer;
	if( sniffer.Run() ) {
		while(1) {
			sleep(3);
		}
	}

	FileLog(SnifferLogFileName, "sniffer退出!");
	return 1;
}
