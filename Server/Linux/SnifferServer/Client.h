/*
 * Client.h
 *
 *  Created on: 2015-11-10
 *      Author: Max
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "SnifferCommandDef.h"
#include "LogManager.h"

#include <common/Arithmetic.hpp>
#include <common/Buffer.h>
#include <common/KCond.h>

#include <string>
using namespace std;

class Client;
class ClientCallback {
public:
	virtual ~ClientCallback(){};
	virtual void OnParseCmd(Client* client, SCMD* scmd) = 0;
};

class Client {
public:
	Client();
	virtual ~Client();

	void SetClientCallback(ClientCallback* pClientCallback);

	/**
	 * 解析数据
	 */
	int ParseData(char* buffer, int len, int iPacketSeq);

	int AddSeq();

	int fd;
	string deviceId;
	string version;
    string brand;
    string model;
    string phoneNumber;
    string whoami;
    string ip;

private:
    Buffer mBuffer;
    int mSeq;
    KMutex mKMutex;
    ClientCallback* mpClientCallback;

    int miPacketSeq;
    KCond mPacketSeqCond;
};

#endif /* CLIENT_H_ */
