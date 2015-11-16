/*
 * SnifferServer.h
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFERSERVER_H_
#define SNIFFERSERVER_H_

#include "Client.h"
#include "Session.h"
#include "MessageList.h"
#include "TcpServer.h"
#include "DataHttpParser.h"

#include <json/json.h>
#include <common/ConfFile.hpp>
#include <common/KSafeMap.h>
#include <common/TimeProc.hpp>
#include <common/StringHandle.h>

#include <map>
#include <list>
using namespace std;

typedef KSafeMap<int, Client*> ClientMap;

// 外/内部服务交互会话
typedef KSafeMap<int, Session*> Client2RequestMap;
typedef KSafeMap<int, Session*> Request2ClientMap;

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
	 * 外部服务发起交互请求
	 * @param request	请求号
	 * @param client	客户端号
	 * @param task		任务
	 */
	bool SendRequestMsg2Client(
			const int& request,
			Client* client,
			ITask* task
			);

	/**
	 * 内部服务返回交互请求
	 */
	bool ReturnClientMsg2Request(
			Client* client,
			SCMD* scmd
			);

	/**
	 * 外部服务器关闭会话
	 */
	bool CloseSessionByRequest(const int& request);

	/**
	 * 内部服务器关闭会话
	 */
	bool CloseSessionByClient(Client* client);

	/**
	 * ###################################################
	 * 外部服务器接口处理
	 */

	/**
	 * 获取在线客户端列表
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
	 * 执行客户端命令
	 */
	int SetClientCmd(
			const char* clientId,
			const char* command,
			Message *m
			);

	/**
	 * 获取客户端目录
	 */
	int GetClientDir(
			const char* clientId,
			const char* directory,
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

	/*
	 * 在线客户端
	 */
	ClientMap mClientMap;

	/*
	 * 外/内部服务交互会话
	 */
	Client2RequestMap mClient2RequestMap;
	Request2ClientMap mRequest2ClientMap;

};

#endif /* SNIFFERSERVER_H_ */
