/*
 * DrSnifferServer.h
 *
 *  Created on: 2014年3月12日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef DRTCPSERVERSOCKET_H_
#define DRTCPSERVERSOCKET_H_

#include "../common/DrSocket.h"
#include "../common/DrTcpSocket.h"
#include "../common/DrThread.h"

#include "DrCommandDef.h"
#include "DrSnifferCallback.h"
#include "../json/json/json.h"

#include <list>

typedef struct _ClientInfo {
    DrTcpSocket aSocket;
    string brand;
    string model;
    string phoneNumber;
}ClientInfo;

typedef list<ClientInfo> ClientList;

class DrSnifferServer: public DrSocket {
public:
	DrSnifferServer();
	virtual ~DrSnifferServer();

    void SetCallback(DrSnifferCallback callback);
    DrSnifferCallback GetCallback();
    
	/*
	 * 开始接收客户端连接请求
	 * maxClient:最多可以处理的客户端连接数
	 * iPort:监听端口
	 * ip:监听地址
	 */
    bool Start(int maxClient, unsigned int iPort, string ip = "");
    void Stop();
    bool IsRunning();

    void Accept();
    void WaitForSingleAcceptObject(unsigned int uiTimeoutSecond);
    void SignalSingleAcceptObject();

	/*
	 * 发送命令到客户端
	 */
	bool SendCommand(DrTcpSocket aSocket, SCCMD sccmd);

	/*
	 * 接收客户端命令
	 */
	SSCMD RecvCommand(DrTcpSocket aSocket);
	void WaitForSingleRecvObject(unsigned int uiTimeoutSecond);
	void SignalSingleRecvObject();

    /*
     * 获取在线列表
     * thread safe
     */
	void LockClientList();
	void UnLockClientList();
    ClientList* GetClientOnlineList();

protected:
    void RefreshClientOnlineList();

private:
    DrTcpSocket m_ServerSocket;

    DrMutex m_ListMutex;
    ClientList m_ClientSocketList;

    DrThread* m_pAcceptThread;
    DrCond m_DrCondAccept;

    DrThread* m_pRecvThread;
    DrCond m_DrCondRecv;
    bool m_bRunning;
    
    DrSnifferCallback m_DrSnifferCallback;

};

#endif /* DRTCPSERVERSOCKET_H_ */
