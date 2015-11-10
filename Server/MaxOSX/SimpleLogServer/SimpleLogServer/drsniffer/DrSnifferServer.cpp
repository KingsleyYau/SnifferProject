/*
 * DrSnifferServer.cpp
 *
 *  Created on: 2014年3月12日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "DrSnifferServer.h"

/*
 * 监听线程
 */
class DrAcceptRunnable : public DrRunnable {
public:
	DrAcceptRunnable(DrSnifferServer *pDrTcpServer) {
		m_pDrTcpServer = pDrTcpServer;
	}
	virtual ~DrAcceptRunnable() {
		m_pDrTcpServer = NULL;
	}
protected:
	void onRun() {
		while(1) {
			if(m_pDrTcpServer && m_pDrTcpServer->IsRunning()) {
				m_pDrTcpServer->Accept();
				m_pDrTcpServer->WaitForSingleAcceptObject(5);
			}
			else {
				break;
			}
		}

	}
private:
	DrSnifferServer *m_pDrTcpServer;
};

/*
 * 接收线程
 */
class DrRecvRunnable : public DrRunnable {
public:
	DrRecvRunnable(DrSnifferServer *pDrTcpServer) {
		m_pDrTcpServer = pDrTcpServer;
	}
	virtual ~DrRecvRunnable() {
		m_pDrTcpServer = NULL;
	}
protected:
	void onRun() {
		while(1) {
			if(m_pDrTcpServer && m_pDrTcpServer->IsRunning()) {
				// 接收客户端请求和判断是否掉线
				m_pDrTcpServer->LockClientList();
				ClientList *pList = m_pDrTcpServer->GetClientOnlineList();
				for(ClientList::iterator itr = pList->begin(); itr != pList->end(); itr++) {
					DrTcpSocket aSocket = (*itr).aSocket;

					SSCMD sscmd;
					sscmd = m_pDrTcpServer->RecvCommand(aSocket);
					switch (sscmd.scmdt) {
					case SinfferServerTypeOffLine:{
						// 与客户端连接已经断开
						ILog("jni.DrRecvRunnable.onRun()", "客户端已经下线, (socket:%d %s:%d)", aSocket.getSocket(), aSocket.GetIP().c_str(), aSocket.GetPort());
						// 删除元素后指针继续移动
						pList->erase(itr++);
                        if(m_pDrTcpServer->GetCallback().onRecvCommand) {
                            m_pDrTcpServer->GetCallback().onRecvCommand(aSocket, sscmd);
                        }
					}break;
					case SinfferServerTypeNone:{
						// 心跳
					}break;
					case ExcuteCommandResult:{
						ILog("jni.DrSnifferServer.RecvCommand()", "接收到客户端命令返回, (socket:%d %s:%d)", aSocket.getSocket(), aSocket.GetIP().c_str(), aSocket.GetPort());
						ILog("jni.DrSnifferServer.RecvCommand()", "返回内容:\n%s", sscmd.param.c_str());
                        if(m_pDrTcpServer->GetCallback().onRecvCommand) {
                            m_pDrTcpServer->GetCallback().onRecvCommand(aSocket, sscmd);
                        }
					}break;
                    case SnifferServerTypeClientInfo:{
                        // 获取手机号和手机型号
                        ILog("jni.DrSnifferServer.RecvCommand()", "接收到客户端版本号, (socket:%d %s:%d)", aSocket.getSocket(), aSocket.GetIP().c_str(), aSocket.GetPort());
						ILog("jni.DrSnifferServer.RecvCommand()", "版本号内容:\n%s", sscmd.param.c_str());
                        
                        Json::Value root;
                        Json::Reader reader;
                        reader.parse(sscmd.param, root);
                        
                        (*itr).brand = root[PHONE_INFO_BRAND].asString();
                        (*itr).model = root[PHONE_INFO_MODEL].asString();
                        (*itr).phoneNumber = root[PHONE_INFO_NUMBER].asString();
                        
                        if(m_pDrTcpServer->GetCallback().onRecvCommand) {
                            m_pDrTcpServer->GetCallback().onRecvCommand(aSocket, sscmd);
                        }
                    }break;
					default: {
					}break;
					}
				}
				m_pDrTcpServer->UnLockClientList();
                
            
				// 每30秒检测一次客户端在线状态, 休息多点
				m_pDrTcpServer->WaitForSingleRecvObject(30);
			}
			else {
				break;
			}
		}

	}
private:
	DrSnifferServer *m_pDrTcpServer;
};

DrSnifferServer::DrSnifferServer() {
	// TODO Auto-generated constructor stub
	m_pAcceptThread = new DrThread(new DrAcceptRunnable(this));
	m_pRecvThread = new DrThread(new DrRecvRunnable(this));

	m_bRunning = false;
}

DrSnifferServer::~DrSnifferServer() {
	// TODO Auto-generated destructor stub
	if(m_pAcceptThread != NULL) {
		delete m_pAcceptThread;
		m_pAcceptThread = NULL;
	}

	if(m_pRecvThread != NULL) {
		delete m_pRecvThread;
		m_pRecvThread = NULL;
	}

}
void DrSnifferServer::SetCallback(DrSnifferCallback callback) {
    m_DrSnifferCallback = callback;
}
DrSnifferCallback DrSnifferServer::GetCallback() {
    return m_DrSnifferCallback;
}
bool DrSnifferServer::Start(int maxClient, unsigned int iPort, string ip) {
	if(!m_ServerSocket.Bind(iPort, ip)) {
		return false;
	}

	if(!m_ServerSocket.Listen(maxClient, false)) {
		return false;
	}

	m_bRunning = true;
	if(m_pAcceptThread->start() < 0) {
		ELog("jni.DrSnifferServer.Start()", "开启监听线程错误!");
		return false;
	} else {
		ILog("jni.DrSnifferServer.Start()", "开启监听线程成功!");
	}

	if(m_pRecvThread->start() < 0) {
		ELog("jni.DrSnifferServer.Start()", "开启接收线程错误!");
		return false;
	} else {
		ILog("jni.DrSnifferServer.Start()", "开启接收线程成功!");
	}

	return true;
}
void DrSnifferServer::Stop() {
	m_bRunning = false;

	// 停止监听线程
	SignalSingleAcceptObject();
	if(m_pAcceptThread) {
		m_pAcceptThread->stop();
		ILog("jni.DrSnifferServer.Stop()", "停止监听线程成功!");
	}
	m_ServerSocket.Close();

	// 停止接收线程
	SignalSingleRecvObject();
	if(m_pRecvThread) {
		m_pRecvThread->stop();
		ILog("jni.DrSnifferServer.Stop()", "停止接收线程成功!");
	}

	LockClientList();
	// 关闭当前所有连接(不锁是因为已经停止了监听线程)
	for(ClientList::iterator itr = m_ClientSocketList.begin(); itr != m_ClientSocketList.end(); itr++) {
		DrTcpSocket aSocket = (*itr).aSocket;;
		aSocket.Close();
	}
	m_ClientSocketList.clear();
	UnLockClientList();
}
void DrSnifferServer::Accept() {
	DrTcpSocket clientSocket = m_ServerSocket.Accept(3000, false);
	if(clientSocket.IsConnected()) {
		// 已经接收到客户端请求
		ILog("jni.DrSnifferServer.Accept()", "收到新的客户端上线, %s:%d", clientSocket.GetIP().c_str(), clientSocket.GetPort());
		LockClientList();
        ClientInfo info;
        info.aSocket = clientSocket;
		m_ClientSocketList.push_back(info);
		UnLockClientList();
        
        GetCallback().onNewClientOnline(clientSocket);

        SignalSingleRecvObject();
	}
}
void DrSnifferServer::LockClientList() {
	m_ListMutex.lock();
}
void DrSnifferServer::UnLockClientList() {
	m_ListMutex.unlock();
}
ClientList* DrSnifferServer::GetClientOnlineList() {
	return &m_ClientSocketList;
}
void DrSnifferServer::RefreshClientOnlineList() {

}
void DrSnifferServer::SignalSingleAcceptObject() {
	m_DrCondAccept.lock();
	m_DrCondAccept.signal();
	m_DrCondAccept.unlock();
}
void DrSnifferServer::WaitForSingleAcceptObject(unsigned int uiTimeoutSecond) {
	m_DrCondAccept.lock();
	m_DrCondAccept.timedwait(uiTimeoutSecond);
	m_DrCondAccept.unlock();
}
bool DrSnifferServer::IsRunning() {
	return m_bRunning;
}
/*
 * 发送命令到客户端
 */
bool DrSnifferServer::SendCommand(DrTcpSocket aSocket, SCCMD sccmd) {
	ILog("jni.DrSnifferServer.SendCommand()", "准备发送命令到客户端,(socket:%d %s:%d)", aSocket.getSocket(), aSocket.GetIP().c_str(), aSocket.GetPort());
	bool bFlag = false;

	// 发送命令头
	sccmd.header.iLen = sizeof(SCCMDT) + sccmd.param.length();
	int iLen = aSocket.SendData((char *)&sccmd.header, sizeof(SCMDH));
	if(iLen == sizeof(SCMDH)) {
		char *pBuffer = new char[sccmd.header.iLen + 1];
		char *p = pBuffer;

		// 类型
		memcpy(p, (char *)&sccmd.scmdt, sizeof(SCCMDT));
		p += sizeof(SCCMDT);
		// 参数
		memcpy(p, sccmd.param.c_str(), sccmd.param.length());

		// 发送类型和参数
		iLen = aSocket.SendData(pBuffer, sccmd.header.iLen);
		if(iLen == sccmd.header.iLen) {
			bFlag = true;
		}

		if(pBuffer != NULL) {
			delete[] pBuffer;
		}
	}

	if(!bFlag) {
		// 发送命令失败,移除客户端
        ELog("jni.DrSnifferServer.SendCommand()", "发送命令到客户端,(socket:%d %s:%d)", aSocket.getSocket(), aSocket.GetIP().c_str(), aSocket.GetPort());
	}

    // 通知接收线程可以进行接收
    SignalSingleRecvObject();
    
	return bFlag;
}

/*
 * 接收客户端命令
 */
SSCMD DrSnifferServer::RecvCommand(DrTcpSocket aSocket) {
	SSCMD sscmd;
	sscmd.scmdt = SinfferServerTypeNone;
    bool bAlive = true;
	// 接收命令头
	char HeadBuffer[sizeof(SCMDH) + 1] = {'\0'};
	int iLen = aSocket.RecvData(HeadBuffer, sizeof(SCMDH), true, &bAlive);
	if(bAlive == false) {
		sscmd.scmdt = SinfferServerTypeOffLine;
	}
	else if(iLen == sizeof(SCMDH)) {
		// 接收头成功
		memcpy(&sscmd.header, HeadBuffer, sizeof(SCMDH));

		char *pBuffer = new char[sscmd.header.iLen + 1];
		memset(pBuffer, '\0', sscmd.header.iLen + 1);
		iLen = aSocket.RecvData(pBuffer, sscmd.header.iLen, true, &bAlive);
		if(bAlive == false) {
			sscmd.scmdt = SinfferServerTypeOffLine;
		}
		if(iLen == sscmd.header.iLen) {
			// 接收类型
			memcpy(&sscmd.scmdt, pBuffer, sizeof(SSCMDT));

			// 接收参数
			sscmd.param = pBuffer + sizeof(SSCMDT);
		}
		delete[] pBuffer;
	}
	return sscmd;
}
void DrSnifferServer::WaitForSingleRecvObject(unsigned int uiTimeoutSecond) {
	m_DrCondRecv.lock();
	m_DrCondRecv.timedwait(uiTimeoutSecond);
	m_DrCondRecv.unlock();
}
void DrSnifferServer::SignalSingleRecvObject() {
	m_DrCondRecv.lock();
	m_DrCondRecv.signal();
	m_DrCondRecv.unlock();
}
