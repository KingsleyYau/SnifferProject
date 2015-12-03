/*
 * SnifferCommandDef.h
 *
 *  Created on: 2014年3月6日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERCOMMANDDEF_H_
#define SNIFFERCOMMANDDEF_H_

#pragma pack(push,1)

#define MAX_PARAM_LEN 4096

/*
 * 命令类型
 */
typedef enum SnifferCommandType {
	SinfferTypeNone = 0,
	SinfferTypeVersion = 1,
	SinfferTypeVersionResult = 2,
	SinfferTypeStart = 3,
	SinfferTypeStop = 4,
	ExcuteCommand  = 5,
	ExcuteCommandResult = 6,
	SnifferTypeClientInfo = 7,
	SnifferTypeClientInfoResult = 8,
	SnifferListDir = 9,
	SnifferListDirResult = 10,
	SnifferUploadFile = 11,
	SnifferUploadFileResult = 12,
} SCMDT;

/*
 * 公用命令头
 */
typedef struct SnifferCommandHedaer {
	SCMDT scmdt;		// 类型
	int len;			// 参数长度
	int seq;			// 请求号
	bool bNew;			// 主动发起请求
} SCMDH;

/*
 * 命令
 */
typedef struct SnifferCommand {
	SCMDH header;					// 命令头
	char param[MAX_PARAM_LEN];		// 参数
} SCMD;

typedef enum ProtocolType {
	HTML,
	JSON,
} PROTOCOLTYPE;

/*
 * Json协议定义
 */

#define COMMON_RET						"RET"
#define COMMON_TOTAL 					"TOTAL"
#define COMMON_PAGE_INDEX 				"PAGEINDEX"
#define COMMON_PAGE_SIZE 				"PAGESIZE"
#define COMMON_PROTOCOL_TYPE			"TYPE"
#define COMMON_PROTOCOL_TYPE_JSON		"JSON"

/**
 * 重新加载配置
 */
#define RELOAD					"/RELOAD"

/**
 * 获取在线列表
 */
#define GET_CLIENT_LIST			"/GET_CLIENT_LIST"
#define CLIENT_LIST				"CLIENT_LIST"
#define CLIENT_ID				"CLIENT_ID"

/**
 * 获取在线详细信息
 */
#define GET_CLIENT_INFO			"/GET_CLIENT_INFO"
#define CLIENT_INFO				"CLIENT_INFO"
#define DEVICE_ID				"DEVICE_ID"
#define PHONE_INFO_BRAND 		"PHONE_INFO_BRAND"
#define PHONE_INFO_MODEL 		"PHONE_INFO_MODEL"
#define PHONE_INFO_NUMBER 		"PHONE_INFO_NUMBER"

/**
 * 执行客户端命令
 */
#define SET_CLIENT_CMD			"/SET_CLIENT_CMD"
#define COMMAND					"COMMAND"

/**
 * 获取客户端目录
 */
#define GET_CLIENT_DIR			"/GET_CLIENT_DIR"
#define DIRECTORY				"DIRECTORY"
#define FILE_LIST				"FILE_LIST"
#define D_NAME					"N"
#define D_TYPE					"T"
#define D_SIZE					"S"
#define D_MODE					"M"

/**
 * 上传客户端文件
 */
#define UPLOAD_CLIENT_FILE		"/UPLOAD_CLIENT_FILE"
#define FILEPATH				"FILEPATH"

#define FILEPATH_LOWER			"filepath"
#define UPLOAD_FILE_LOWER		"upload_file"
#define DEVICE_ID_LOWER			"device_id"

#define DOWN_SERVER_FILE		"/download.cgi"
#define DOWN_SERVER_ADDRESS		"SERVER_ADDRESS"

#pragma pack(pop)

#endif /* SNIFFERCOMMANDDEF_H_ */
