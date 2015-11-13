/*
 * Client.h
 *
 *  Created on: 2015-11-10
 *      Author: Max
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
using namespace std;

#include "DataParser.h"
#include "SnifferCommandDef.h"

#include "LogManager.h"
#include <common/Arithmetic.hpp>
#include <common/Buffer.h>

class Client;
class ClientCallback {
public:
	virtual ~ClientCallback(){};
	virtual void OnParseCmd(Client* client, SCMD* scmd) = 0;
};

class Client : public DataParser {
public:
	Client();
	virtual ~Client();

	void SetClientCallback(ClientCallback* pClientCallback);

	/**
	 * 解析数据
	 */
	int ParseData(char* buffer, int len);

	/**
	 * 发送命令
	 */
	bool SendCmd(SCMD* scmd, int seq = 0, bool bNew = true);

	int fd;
	string deviceId;
    string brand;
    string model;
    string phoneNumber;

private:
    Buffer mBuffer;
    int mSeq;
    KMutex mKMutex;
    ClientCallback* mpClientCallback;
};

#endif /* CLIENT_H_ */
