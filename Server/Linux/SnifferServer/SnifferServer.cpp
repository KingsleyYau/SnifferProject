/*
 * SnifferServer.cpp
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "SnifferServer.h"
#include "TimeProc.hpp"

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
			printf("# Match Server can not load config file exit. \n");
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

	bool bFlag = false;

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
	Client *client = new Client();
	client->fd = m->fd;

	if( ts == &mClientTcpServer ) {
		mClientMap.Lock();
		mClientMap.Insert(m->fd, client);
		mClientMap.Unlock();
	}

	return true;
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

				// Process finish, send respond
				ts->SendMessageByQueue(sm);
			} else {
				// receive continue
				ts->GetIdleMessageList()->PushBack(sm);
			}
		} else if( &mClientTcpInsideServer == ts ){
			// 内部服务请求
			ret = HandleInsideRecvMessage(m, sm);

			if( ret != 0 ) {
				// Process finish, send respond
				ts->SendMessageByQueue(sm);
			}
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

/**
 * OnDisconnect
 */
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
		mClientMap.Lock();
		ClientMap::iterator itr = mClientMap.Erase(fd);
		delete itr->second;
		mClientMap.Unlock();
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

int SnifferServer::HandleRecvMessage(Message *m, Message *sm) {
	int ret = -1;
	string param;

	Json::FastWriter writer;
	Json::Value rootSend;

	if( m == NULL ) {
		return ret;
	}

	Client *client = NULL;
	if ( DiffGetTickCount(m->starttime, GetTickCount()) < miTimeout * 1000 ) {
		if( m->buffer != NULL ) {
			mClientMap.Lock();
			ClientMap::iterator itr = mClientMap.Find(m->fd);
			if( itr != mClientMap.End() ) {
				client = itr->second;
			}
			mClientMap.Unlock();

			if( client != NULL ) {
				ret = client->ParseData(m->buffer, m->len);
			}
		}
	} else {
		param = writer.write(rootSend);
	}

	if( ret == 1 && client != NULL ) {
		ret = 0;
		SSCMD sscmd = client->sscmd;

		switch (sscmd.scmdt) {
		case SnifferServerTypeClientInfo:{
			// 获取手机号和手机型号
            Json::Value root;
            Json::Reader reader;
            reader.parse(sscmd.param, root);

            client->brand = root[PHONE_INFO_BRAND].asString();
            client->model = root[PHONE_INFO_MODEL].asString();
            client->phoneNumber = root[PHONE_INFO_NUMBER].asString();

    		LogManager::GetLogManager()->Log(LOG_WARNING,
    				"SnifferServer::HandleRecvMessage( "
    				"tid : %d, "
    				"m->fd : [%d], "
    				"[获取手机号和手机型号], "
    				"param : %s "
    				")",
    				(int)syscall(SYS_gettid),
    				m->fd,
    				param.c_str()
    				);
		}break;
		default:break;
		}
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
//
//	snprintf(sm->buffer, MAXLEN - 1, "%s",
//			param.c_str()
//			);
//	sm->len = strlen(sm->buffer);

	return ret;
}

int SnifferServer::HandleTimeoutMessage(Message *m, Message *sm) {
	int ret = -1;

	Json::FastWriter writer;
	Json::Value rootSend, womanListNode, womanNode;

	unsigned int iHandleTime = 0;

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
	string param;

	Json::FastWriter writer;
	Json::Value rootSend, womanListNode, womanNode;

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
				"pPath : %s, "
				"parse "
				")",
				(int)syscall(SYS_gettid),
				m->fd,
				type,
				pPath
				);

		if( type == GET ) {
			if( strcmp(pPath, "/GET_ONLINE_LIST") == 0 ) {
				LogManager::GetLogManager()->Log(
						LOG_MSG,
						"SnifferServer::HandleInsideRecvMessage( "
						"tid : %d, "
						"m->fd : [%d], "
						"GET_ONLINE_LIST "
						")",
						(int)syscall(SYS_gettid),
						m->fd
						);

				Json::Value clientListNode;
				if( 1 == GetOnlineList(clientListNode, m) ) {
					rootSend["clientList"] = clientListNode;
				}

				rootSend["ret"] = 1;
				param = writer.write(rootSend);
				ret = 1;

			} else if( strcmp(pPath, "/RELOAD") == 0 ) {
				LogManager::GetLogManager()->Log(
						LOG_MSG,
						"SnifferServer::HandleInsideRecvMessage( "
						"tid : %d, "
						"m->fd : [%d], "
						"RELOAD "
						")",
						(int)syscall(SYS_gettid),
						m->fd
						);
				if( Reload()) {
					rootSend["ret"] = 1;
				} else {
					rootSend["ret"] = 0;
				}

				param = writer.write(rootSend);
				ret = 1;

			} else {
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
int SnifferServer::GetOnlineList(
		Json::Value& listNode,
		Message *m
		) {
	LogManager::GetLogManager()->Log(
			LOG_STAT,
			"SnifferServer::GetOnlineList( "
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
		clientNode["fd"] = client->fd;
		clientNode[PHONE_INFO_BRAND] = client->brand;
		clientNode[PHONE_INFO_MODEL] = client->model;
		clientNode[PHONE_INFO_NUMBER] = client->phoneNumber;
		listNode.append(clientNode);
	}
	mClientMap.Unlock();

	return 1;
}
