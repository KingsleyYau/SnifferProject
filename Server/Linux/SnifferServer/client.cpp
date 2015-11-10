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
    sscmd.scmdt = SinfferServerTypeNone;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
}

int Client::ParseData(char* buffer, int len)  {
	int ret = 0;

	int recvLen = (len < (MAXLEN - 1) - message.len)?len:(MAXLEN - 1) - message.len;
	if( recvLen > 0 ) {
		memcpy(message.buffer + message.len, buffer, recvLen);
		message.len += recvLen;
		message.buffer[message.len + 1] = '\0';
	}

	// 接收命令头
	if(message.len >= sizeof(SCMDH)) {
		// 接收头成功
		memcpy(&sscmd.header, message.buffer, sizeof(SCMDH));

		int iLen = message.len - sizeof(SCMDH);
		if( iLen == sscmd.header.iLen ) {
			// 接收命令完成
			// 接收类型
			memcpy(&sscmd.scmdt, message.buffer + sizeof(SCMDH), sizeof(SSCMDT));

			// 接收参数
			sscmd.param = message.buffer + sizeof(SCMDH) + sizeof(SSCMDT);

			message.len = 0;
			ret = 1;
		}
	}

	return ret;
}
