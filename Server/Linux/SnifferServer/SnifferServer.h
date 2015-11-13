/*
 * SnifferServer.h
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERSERVER_H_
#define SNIFFERSERVER_H_

#include "MessageList.h"
#include "TcpServer.h"
#include "DataHttpParser.h"
#include "Client.h"

#include <json/json.h>
#include <common/ConfFile.hpp>
#include <common/KSafeMap.h>

#include <map>
#include <list>
using namespace std;

typedef KSafeMap<int, Client*> ClientMap;

typedef KSafeMap<int, int> Client2RequestMap;
typedef KSafeMap<int, int> Request2ClientMap;

class StateRunnable;
class SnifferServer : public TcpServerObserver, ClientCallback {
public:
	SnifferServer();
	virtual ~SnifferServer();

	void Run(const string& config);
	void Run();
	bool Reload();
	bool IsRunning();

	/* callback by TcpServerObserver */
	bool OnAccept(TcpServer *ts, Message *m);
	void OnRecvMessage(TcpServer *ts, Message *m);
	void OnSendMessage(TcpServer *ts, Message *m);
	void OnDisconnect(TcpServer *ts, int fd);
	void OnTimeoutMessage(TcpServer *ts, Message *m);

	void StateRunnableHandle();

	/**
	 * Implement from ClientCallback
	 */
	void OnParseCmd(Client* client, SCMD* scmd);

private:
	/*
	 *	请求解析函数
	 *	return : -1:Send fail respond / 0:Continue recv, send no respond / 1:Send OK respond
	 */
	int HandleRecvMessage(Message *m, Message *sm);
	int HandleTimeoutMessage(Message *m, Message *sm);
	int HandleInsideRecvMessage(Message *m, Message *sm);

	/**
	 * 内部服务器主动发起的交互请求
	 */
	bool SendRequestMsg2Client(
			const int& request,
			const int& client,
			const SnifferCommandType& type,
			const char* buffer,
			const int& len
			);

	bool ReturnClientMsg2Request(
			const int& client,
			const int& code,
			const char* reason,
			const char* buffer,
			const int& len
			);

	/**
	 * 客户端主动发起的交互请求
	 */
	bool ReturnRequestMsg2Client(
			const int& request,
			const SnifferCommandType& type,
			const char* buffer,
			const int& len
			);

	/**
	 * 获取在线客户端
	 */
	int GetClientList(
			Json::Value& listNode,
			Message *m
			);

	/**
	 * 获取在线客户端详细信息
	 */
	int GetClientInfo(
			Json::Value& clientNode,
			const char* clientId,
			Message *m
			);

	/**
	 * 对指定客户端运行命令
	 */
	int SetClientCmd(
			const char* clientId,
			const char* command,
			Message *m
			);

	TcpServer mClientTcpServer;
	TcpServer mClientTcpInsideServer;

	/**
	 * 配置文件锁
	 */
	KMutex mConfigMutex;

	// BASE
	short miPort;
	int miMaxClient;
	int miMaxHandleThread;
	int miMaxQueryPerThread;
	/**
	 * 请求超时(秒)
	 */
	unsigned int miTimeout;

	// LOG
	int miLogLevel;
	string mLogDir;
	int miDebugMode;

	/**
	 * 是否运行
	 */
	bool mIsRunning;

	/**
	 * State线程
	 */
	StateRunnable* mpStateRunnable;
	KThread* mpStateThread;

	/**
	 * 统计请求
	 */
	unsigned int mTotal;
	unsigned int mHit;
	unsigned int mResponed;
	KMutex mCountMutex;

	/**
	 * 配置文件
	 */
	string mConfigFile;

	/**
	 * 监听线程输出间隔
	 */
	unsigned int miStateTime;

	ClientMap mClientMap;

	Client2RequestMap mClient2RequestMap;
	Request2ClientMap mRequest2ClientMap;
};

#endif /* SNIFFERSERVER_H_ */
