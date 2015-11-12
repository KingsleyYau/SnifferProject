/*
 * Client.cpp
 *
 *  Created on: 2015-11-10
 *      Author: Max
 */

#include "Client.h"

Client::Client() {
	// TODO Auto-generated constructor stub
	fd = -1;

    brand = "";
    model = "";
    phoneNumber = "";

	type = 0;
	message.Reset();
}

Client::~Client() {
	// TODO Auto-generated destructor stub
	while( !cmdListRecv.Empty() ) {
		SCMD *scmd = cmdListRecv.PopFront();
		delete scmd;
	}
}

int Client::ParseData(char* buffer, int len)  {
	int ret = -1;

	int recvLen = (len < (MAXLEN - 1) - message.len)?len:(MAXLEN - 1) - message.len;
	if( recvLen > 0 ) {
		memcpy(message.buffer + message.len, buffer, recvLen);
		message.len += recvLen;
		message.buffer[message.len] = '\0';
	}

	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"Client::ParseData( "
			"tid : %d, "
			"message.len : %d "
			")",
			(int)syscall(SYS_gettid),
			message.len
			);

	// 解析命令头
	while(message.len > sizeof(SCMDH)) {
		SCMDH *header = (SCMDH*)message.buffer;

		LogManager::GetLogManager()->Log(
				LOG_STAT,
				"Client::ParseData( "
				"tid : %d, "
				"header->scmdt : %d, "
				"header->len : %d, "
				"header->bNew : %s, "
				"header->seq : %d "
				")",
				(int)syscall(SYS_gettid),
				header->scmdt,
				header->len,
				header->bNew?"true":"false",
				header->seq
				);

		int iLen = message.len - sizeof(SCMDH);
		if( iLen >= header->len ) {
			if( header->len > 0 ) {
				// 接收参数
				SCMD *scmd = new SCMD();
				cmdListRecv.PushBack(scmd);
				memcpy(scmd, message.buffer, sizeof(SCMDH) + header->len);
				scmd->param[header->len] = '\0';

				LogManager::GetLogManager()->Log(
						LOG_STAT,
						"Client::ParseData( "
						"tid : %d, "
						"scmd->param : %s "
						")",
						(int)syscall(SYS_gettid),
						scmd->param
						);
			}

			// 替换数据
			message.len -= sizeof(SCMDH) + header->len;
			memcpy(message.buffer, message.buffer + sizeof(SCMDH) + header->len, message.len);

			ret = 1;
		} else {
			break;
		}
	}

	return ret;
}
