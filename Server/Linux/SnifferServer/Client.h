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
#include "MessageList.h"

#include <common/Arithmetic.hpp>
#include <common/Buffer.h>
#include <common/KCond.h>
#include <common/KSafeMap.h>

#include <string>
using namespace std;

class Client;
class ClientCallback {
public:
	virtual ~ClientCallback(){};
	virtual void OnParseCmd(Client* client, SCMD* scmd) = 0;
};

typedef KSafeMap<int, Message*> MessageMap;

class Client {
public:
	Client();
	virtual ~Client();

	void SetClientCallback(ClientCallback* pClientCallback);

	/**
	 * 解析数据
	 */
	int ParseData(Message* m);

	/**
	 * 增加发送协议包序列号
	 */
	int AddSeq();

	int fd;
	bool isOnline;
	string deviceId;
	string version;
    string ip;
    string cwd;

    string brand;
    string model;
    string phoneNumber;
    string abi;
    bool isRoot;
    string screencap;

private:
    int ParseDataNoCache(Message* m);

    void ParseMessageInCache();

    /**
     * 组包成功回调
     */
    ClientCallback* mpClientCallback;

    /**
     * 多线程组包/解包互斥锁
     */
    KMutex mKMutex;

    /**
     * 未组成协议包的临时buffer
     */
    Buffer mBuffer;

    /**
     * 已经发送的协议包序列号
     */
    int miPacketSendSeq;

    /**
     * 需要接收的数据包序列号
     */
    int miDataPacketRecvSeq;

	MessageList mIdleMessageList;
	MessageMap mMessageMap;

};

#endif /* CLIENT_H_ */
