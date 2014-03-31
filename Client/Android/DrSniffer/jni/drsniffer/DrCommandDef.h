/*
 * DrCommandHandler.h
 *
 *  Created on: 2014年3月6日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef DRCOMMANDHANDLER_H_
#define DRCOMMANDHANDLER_H_

#include <string>
using namespace std;

#define ServerAddress "drsniffer.wicp.net"
#define ServerPort 8765

/*
 * 公用命令头
 */
typedef struct SnifferCommandHedaer {
	int iLen;			// 类型和参数长度
}SCMDH;

/*
 * Client命令类型
 */
typedef enum SnifferClientCommandType {
	SinfferClientTypeNone = 0,
	SinfferClientTypeVersion = 1,
	SinfferClientTypeStart = 2,
	SinfferClientTypeStop = 3,
	ExcuteCommand = 4,
}SCCMDT;

/*
 * Client可以处理的命令
 */
typedef struct SnifferClientCommand {
	SnifferCommandHedaer header;
	SCCMDT scmdt;		// 类型
	string param;		// 参数
}SCCMD;

/*
 * Server命令类型
 */
typedef enum SnifferServerCommandType{
	SinfferServerTypeOffLine = 0,
	SinfferServerTypeNone = 1,
	ExcuteCommandResult = 2,
	SnifferServerTypeClientInfo = 3,
}SSCMDT;

/*
 * Server可以处理的命令
 */
typedef struct SnifferServerCommand {
	SnifferCommandHedaer header;
	SSCMDT scmdt;		// 类型
	string param;		// 参数
}SSCMD;

/*
 * Json协议定义
 */
#define PHONE_INFO_BRAND "phone_info_brand"
#define PHONE_INFO_MODEL "phone_info_model"
#define PHONE_INFO_NUMBER "phone_info_number"

#endif /* DRCOMMANDHANDLER_H_ */
