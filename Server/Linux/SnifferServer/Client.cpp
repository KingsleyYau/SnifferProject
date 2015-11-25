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

	deviceId = "";
    brand = "";
    model = "";
    phoneNumber = "";

    mSeq = 0;
	mBuffer.Reset();
	mpClientCallback = NULL;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
}

void Client::SetClientCallback(ClientCallback* pClientCallback) {
	mpClientCallback = pClientCallback;
}

int Client::ParseData(char* buffer, int len)  {
	int ret = 0;

	mKMutex.lock();

	int recvLen = (len < MAXLEN - mBuffer.len)?len:(MAXLEN - mBuffer.len);
	if( recvLen > 0 ) {
		LogManager::GetLogManager()->Log(
				LOG_STAT,
				"Client::ParseData( "
				"tid : %d, "
				"mBuffer.len : %d, "
				"recvLen : %d "
				")",
				(int)syscall(SYS_gettid),
				mBuffer.len,
				recvLen
				);

		memcpy(mBuffer.buffer + mBuffer.len, buffer, recvLen);
		mBuffer.len += recvLen;
	}

	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"Client::ParseData( "
			"tid : %d, "
			"mBuffer.len : %d "
			")",
			(int)syscall(SYS_gettid),
			mBuffer.len
			);

	// 解析命令头
	while(mBuffer.len >= sizeof(SCMDH)) {
		// 头部已经收够
		SCMDH *header = (SCMDH*)mBuffer.buffer;

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

		int iLen = mBuffer.len - sizeof(SCMDH);
		if( iLen >= header->len ) {
			// 参数已经收够
			SCMD scmd;
			memcpy(&scmd, mBuffer.buffer, sizeof(SCMDH) + header->len);

			LogManager::GetLogManager()->Log(
					LOG_STAT,
					"Client::ParseData( "
					"tid : %d, "
					"scmd->param : \n%s "
					")",
					(int)syscall(SYS_gettid),
					scmd.param
					);

			if( mpClientCallback != NULL ) {
				mpClientCallback->OnParseCmd(this, &scmd);
			}

			// 替换数据
			mBuffer.len -= sizeof(SCMDH) + header->len;

			LogManager::GetLogManager()->Log(
					LOG_STAT,
					"Client::ParseData( "
					"tid : %d, "
					"mBuffer.len : %d "
					")",
					(int)syscall(SYS_gettid),
					mBuffer.len
					);

			if( mBuffer.len > 0 ) {
				memcpy(mBuffer.buffer, mBuffer.buffer + sizeof(SCMDH) + header->len, mBuffer.len);
			}

			ret = 1;
		} else {
			break;
		}
	}

	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"Client::ParseData( "
			"tid : %d, "
			"ret : %d "
			")",
			(int)syscall(SYS_gettid),
			ret
			);

	mKMutex.unlock();

	return ret;
}

int Client::AddSeq() {
	int seq;
	mKMutex.lock();
	seq = mSeq++;
	mKMutex.unlock();
	return seq;
}
