/*
 * DrTcpSocket.cpp
 *
 *  Created on: 2014年2月24日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "DrTcpSocket.h"
#include "Arithmetic.h"

DrTcpSocket::DrTcpSocket() {
	// TODO Auto-generated constructor stub
	m_sAddress = "";
	m_iPort = 0;
	m_bConnected = false;
}

DrTcpSocket::~DrTcpSocket() {
	// TODO Auto-generated destructor stub
}

DrTcpSocket DrTcpSocket::operator=(const DrTcpSocket &obj) {
//	DLog("jni.DrTcpSocket::operator=", "obj:(%p)", &obj);
	this->m_Socket = obj.m_Socket;
	this->m_sAddress = obj.m_sAddress;
	this->m_iPort = obj.m_iPort;
	this->m_bConnected = obj.m_bConnected;
	this->m_bBlocking = obj.m_bBlocking;
	return *this;
}

void DrTcpSocket::SetConnnected() {
	m_bConnected = true;
}

bool DrTcpSocket::IsConnected() {
	return m_bConnected;
}
void DrTcpSocket::SetAddress(struct sockaddr_in addr) {
	m_sAddress = DrSocket::IpToString(addr.sin_addr.s_addr);
	m_iPort = ntohs(addr.sin_port);
}
string DrTcpSocket::GetIP() {
	return m_sAddress;
}
int DrTcpSocket::GetPort() {
	return m_iPort;
}

/*
 * Client
 */
/*
 * 每次都是新的socket，所以不锁
 */
int DrTcpSocket::Connect(string strAddress, unsigned int uiPort, bool bBlocking) {
	m_sAddress = strAddress;
	m_iPort = uiPort;

	int iRet = -1, iFlag = 1;
	struct sockaddr_in dest;
	hostent* hent = NULL;
	if ((m_Socket = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
		DLog("jni.DrTcpSocket::Connect()", "创建socket(%d)成功", m_Socket);
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(uiPort);

		dest.sin_addr.s_addr = inet_addr((const char*)strAddress.c_str());
		if (dest.sin_addr.s_addr == -1L) {
			if ((hent = gethostbyname((const char*)strAddress.c_str())) != NULL) {
				dest.sin_family = hent->h_addrtype;
				memcpy((char*)&dest.sin_addr, hent->h_addr, hent->h_length);
				m_sAddress = DrSocket::IpToString(dest.sin_addr.s_addr);
				DLog("jni.DrTcpSocket::Connect()", "域名转IP成功 域名:%s ip:%s",
										(const char*)strAddress.c_str(), m_sAddress.c_str());
			} else {
				ELog("jni.DrTcpSocket::Connect()", "域名转IP失败 域名:%s 原因:%s",
						(const char*)strAddress.c_str(), hstrerror(h_errno));
				iRet = -1;
				goto EXIT_ERROR_TCP;
			}
		}

		// 不使用发送缓冲区
		setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
		// 接收缓冲区大小
		iFlag = 1500;
		setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char *)&iFlag, sizeof(iFlag));
		// socket可重用
		iFlag = 1;
		setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&iFlag, sizeof(iFlag));

		if(setBlocking(true)) {
			// 设置阻塞
		}
		else {
			iRet = -1;
			ELog("jni.DrTcpSocket::Connect()", "Connect前设置socket阻塞失败！");
			goto EXIT_ERROR_TCP;
		}

		DLog("jni.DrTcpSocket::Connect()", "开始连接(%s:%d)", m_sAddress.c_str(), uiPort);
		if (connect(m_Socket, (struct sockaddr *)&dest, sizeof(dest)) != -1) {
			DLog("jni.DrTcpSocket::Connect()", "连接成功，开始设置socket属性...");
			if(setBlocking(bBlocking)) {
				// 设置阻塞
			}
			else {
				iRet = -1;
				ELog("jni.DrTcpSocket::Connect()", "Connect后设置socket阻塞属性失败!");
				goto EXIT_ERROR_TCP;
			}

			// 设置心跳检测
		    /*deal with the tcp keepalive
		      iKeepAlive = 1 (check keepalive)
		      iKeepIdle = 600 (active keepalive after socket has idled for 10 minutes)
		      KeepInt = 60 (send keepalive every 1 minute after keepalive was actived)
		      iKeepCount = 3 (send keepalive 3 times before disconnect from peer)
		     */
		    int iKeepAlive = 1, iKeepIdle = 15, KeepInt = 15, iKeepCount = 2;
		    if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (void*)&iKeepAlive, sizeof(iKeepAlive)) < 0) {
		    	iRet = -1;
		    	ELog("jni.DrTcpSocket::Connect()", "设置iKeepAlive失败！");
		    	goto EXIT_ERROR_TCP;
		    }
            if (setsockopt(m_Socket, IPPROTO_TCP, TCP_KEEPALIVE, (void *)&iKeepIdle, sizeof(iKeepIdle)) < 0) {
                iRet = -1;
                ELog("jni.DrTcpSocket::Accept()", "设置TCP_KEEPINTVL失败！");
                goto EXIT_ERROR_TCP;
            }
		    if (setsockopt(m_Socket, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&KeepInt, sizeof(KeepInt)) < 0) {
		    	iRet = -1;
		    	ELog("jni.DrTcpSocket::Connect()", "设置KeepInt失败！");
		    	goto EXIT_ERROR_TCP;
		    }
		    if (setsockopt(m_Socket, IPPROTO_TCP, TCP_KEEPCNT, (void *)&iKeepCount, sizeof(iKeepCount)) < 0) {
		    	iRet = -1;
		    	ELog("jni.DrTcpSocket::Connect()", "设置iKeepCount失败！");
		    	goto EXIT_ERROR_TCP;
		    }
		    DLog("jni.DrTcpSocket::Connect()", "设置tcp心跳检测成功，每次%d秒检测一次，连续%d次没有收到心跳则断开，连接成功后%d秒开始心跳检测！", \
		    		KeepInt, iKeepCount, iKeepIdle);

		    iRet = 1;
		}
		else {
			ELog("jni.DrTcpSocket::Connect()", "连接失败！");
		}
	}
	else {
		ELog("jni.DrTcpSocket::Connect()", "创建socket失败！");
	}

EXIT_ERROR_TCP:
	if (iRet != 1) {
		Close();
	}
	else {
		m_bConnected = true;
	}
	return iRet;
}
int DrTcpSocket::SendData(char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout) {
	m_SocketMutex.lock();
	unsigned int uiBegin = GetTick();
	int iOrgLen = uiSendLen;
	int iRet = -1, iRetS = -1;
	int iSendedLen = 0;
	fd_set wset;

	string log = Arithmetic::AsciiToHexWithSep(pBuffer, uiSendLen);
	DLog("jni.DrTcpSocket::SendData", "准备向(socket:%d %s:%d)发送数据，Hex编码后:\n%s(%d)", m_Socket, m_sAddress.c_str(), m_iPort, log.c_str(), uiSendLen);
    
    if(!IsConnected()) {
        goto EXIT_TCP_SEND;
    }

	if(m_bBlocking) {
		// 阻塞
		iRet = send(m_Socket, pBuffer, uiSendLen, 0);
		goto EXIT_TCP_SEND;
	}
	else {
		// 不阻塞
		while (true) {
			struct timeval tout;
			tout.tv_sec = uiTimeout / 1000;
			tout.tv_usec = (uiTimeout % 1000) * 1000;
			FD_ZERO(&wset);
			FD_SET(m_Socket, &wset);
			iRetS = select(m_Socket + 1, NULL, &wset, NULL, &tout);
			if (iRetS == -1) {
				iRet = -1;
				goto EXIT_TCP_SEND;
			} else if (iRetS == 0) {
				iRet = iSendedLen;
				goto EXIT_TCP_SEND;
			}

			iRet = send(m_Socket, pBuffer, uiSendLen, 0);
			if (iRet == -1 || (iRetS == 1 && iRet == 0)) {
				usleep(100);
				if (EWOULDBLOCK == errno) {
					if (IsTimeout(uiBegin, uiTimeout)) {
						iRet = iSendedLen;
						goto EXIT_TCP_SEND;
					}
					continue;
				} else {
					if (iSendedLen == 0){
						iRet = -1;
						goto EXIT_TCP_SEND;
					}
					iRet = iSendedLen;
					goto EXIT_TCP_SEND;
				}
			} else if (iRet == 0) {
				iRet = iSendedLen;
				goto EXIT_TCP_SEND;
			}
			pBuffer += iRet;
			iSendedLen += iRet;
			uiSendLen -= iRet;
			if (iSendedLen >= iOrgLen) {
				iRet = iSendedLen;
				goto EXIT_TCP_SEND;
			}
			if (IsTimeout(uiBegin, uiTimeout)) {
				iRet = iSendedLen;
				goto EXIT_TCP_SEND;
			}
		}
	}

EXIT_TCP_SEND:
	m_SocketMutex.unlock();
	if(iRet == -1) {
		DLog("jni.DrTcpSocket::SendData", "发送失败!");
		Close();
	}
	else {
		DLog("jni.DrUdpSocket::SendData", "成功发送字节数:(%d)", iRet);
	}
	return iRet;
}

int DrTcpSocket::RecvData(char* pBuffer, unsigned int uiRecvLen, bool bRecvAll, bool* pbAlive, unsigned int uiTimeout) {
	m_SocketMutex.lock();
	unsigned int uiBegin = GetTick();
	int iOrgLen = uiRecvLen;
	int iRet = -1, iRetS = -1;
	int iRecvedLen = 0;
    bool bAlive = true;
	fd_set rset;

    if(!IsConnected()) {
        bAlive = false;
        goto EXIT_TCP_RECV;
    }
    
	if(m_bBlocking) {
		// 阻塞
		iRet = recv(m_Socket, pBuffer, uiRecvLen, 0);
		if(iRet > 0) {
			string log = Arithmetic::AsciiToHexWithSep(pBuffer, iRet);
			DLog("jni.DrTcpSocket::RecvData()", "收到来自(阻塞socket:%d %s:%d)访问，成功接收字节数:(%d) Hex编码后:\n%s", m_Socket, m_sAddress.c_str(), m_iPort, iRet, log.c_str());
		}
        else {
            bAlive = false;
        }
		goto EXIT_TCP_RECV;
	}
	else {
		// 不阻塞
		while (true) {
			timeval tout;
			tout.tv_sec = uiTimeout / 1000;
			tout.tv_usec = (uiTimeout % 1000) * 1000;
			FD_ZERO(&rset);
			FD_SET(m_Socket, &rset);
			iRetS = select(m_Socket + 1, &rset, NULL, NULL, &tout);
			if (iRetS == -1) {
				ELog("Jni.DrTcpSocket::RecvData()", "(socket:%d)没有任何可读取数据, 连接已经断开!", m_Socket);
				iRet = -1;
                bAlive = false;
				goto EXIT_TCP_RECV;
			} else if (iRetS == 0) {
				DLog("Jni.DrTcpSocket::RecvData()", "(socket:%d) %d秒超时没有数据返回, 已经收到:%d字节", m_Socket,  uiTimeout / 1000, iRecvedLen);
				iRet = iRecvedLen;
				goto EXIT_TCP_RECV;
			}
            else {
                DLog("Jni.DrTcpSocket::RecvData()", "iRetS > 0");
                iRet = recv(m_Socket, pBuffer, uiRecvLen, 0);
                if(iRet == 0) {
                    // 对方closesocket，发来byebye
                    ELog("Jni.DrTcpSocket::RecvData()", "客户端主动断开连接!");
                    iRet = iRecvedLen;
                    bAlive = false;
                    goto EXIT_TCP_RECV;
                }
                else if (iRet == -1){
                    // 检测是否被动断开连接
                    usleep(1000);
                    DLog("Jni.DrTcpSocket::RecvData()", "errno:%d", errno);
                    if (EWOULDBLOCK == errno || EINTR == errno){
                        DLog("Jni.DrTcpSocket::RecvData()", "EWOULDBLOCK || EINTR(超时没有数据可读!)");
                        if (IsTimeout(uiBegin, uiTimeout)){
                            iRet = iRecvedLen;
                            goto EXIT_TCP_RECV;
                        }
                        continue;
                    } else {
                        // 客户端网络异常断开连接
                        ELog("Jni.DrTcpSocket::RecvData()", "客户端异常断开连接!");
                        iRet = iRecvedLen;
                        bAlive = false;
                        goto EXIT_TCP_RECV;
                    }
                }
                
                string log = Arithmetic::AsciiToHexWithSep(pBuffer, iRet);
                DLog("jni.DrTcpSocket::RecvData()", "收到来自(非阻塞socket:%d %s:%d)访问，成功接收字节数:(%d) Hex编码后:\n%s", m_Socket, m_sAddress.c_str(), m_iPort, iRet, log.c_str());
                
                pBuffer += iRet;
                iRecvedLen += iRet;
                uiRecvLen -= iRet;
                if (iRecvedLen >= iOrgLen) {
                    iRet = iRecvedLen;
                    goto EXIT_TCP_RECV;
                }
                if (!bRecvAll) {
                    iRet = iRecvedLen;
                    goto EXIT_TCP_RECV;
                }
                if (IsTimeout(uiBegin, uiTimeout)){
                    iRet = iRecvedLen;
                    goto EXIT_TCP_RECV;
                }
            }

		}
	}

EXIT_TCP_RECV:
	m_SocketMutex.unlock();
	if(m_bBlocking && bAlive == false) {
		DLog("jni.DrTcpSocket::RecvData()", "阻塞tcp连接(socket:%d %s:%d)已经断开!", m_Socket, m_sAddress.c_str(), m_iPort);
		Close();
	}
	else if(iRet == -1 || bAlive == false) {
		DLog("jni.DrTcpSocket::RecvData()", "非阻塞tcp连接(socket:%d %s:%d)已经断开!", m_Socket, m_sAddress.c_str(), m_iPort);
		Close();
	}
    if(pbAlive != NULL) {
        *pbAlive = bAlive;
    }
	return iRet;
}

/*
 * Server
 */
bool DrTcpSocket::Bind(unsigned int iPort, string ip) {
	bool bFlag = false;
	if((m_Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		// create socket error;
		ELog("jni.DrTcpSocket.Bind()", "创建socket错误!");
		bFlag = false;
		goto EXIT_TCP_BIND;
	}
	else {
		DLog("jni.DrTcpSocket.Bind()", "创建socket(%d)成功!", m_Socket);
	}

	struct sockaddr_in localAddr;
	bzero(&localAddr, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	if(ip.length() > 0) {
		inet_aton(ip.c_str(), &localAddr.sin_addr);
	}
	else {
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	localAddr.sin_port = htons(iPort);
	if(bind(m_Socket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
		// bind socket error
		ELog("jni.DrTcpSocket.Bind()", "绑定(%s:%d)错误!", DrSocket::IpToString(localAddr.sin_addr.s_addr).c_str(), iPort);
		bFlag = false;
		goto EXIT_TCP_BIND;
	}
	else {
		DLog("jni.DrTcpSocket.Bind()", "绑定端口(%s:%d)成功!", DrSocket::IpToString(localAddr.sin_addr.s_addr).c_str(), iPort);
		bFlag = true;
	}

EXIT_TCP_BIND:
	if(!bFlag) {
		Close();
	}
    return bFlag;
}

bool DrTcpSocket::Listen(int maxSocketCount, bool bBlocking) {
	bool bFlag = false;
    if (listen(m_Socket, maxSocketCount) == -1) {
        ELog("jni.DrTcpSocket.Listen()", "监听socket错误!");
		bFlag = false;
		goto EXIT_TCP_LISTEN;
    }
    else {
		DLog("jni.DrTcpSocket.Start()", "监听最大队列(%d)成功!", maxSocketCount);
		bFlag = true;
	}

    m_bBlocking = bBlocking;
EXIT_TCP_LISTEN:
	if(!bFlag) {
		Close();
	}
	return bFlag;
}
DrTcpSocket DrTcpSocket::Accept(unsigned int uiTimeout, bool bBlocking) {
	m_SocketMutex.lock();
    DrTcpSocket clientSocket;
    socket_type accpetSocket = -1;

    struct sockaddr_in remoteAddr;
	bzero(&remoteAddr, sizeof(remoteAddr));
	socklen_t iRemoteAddrLen = sizeof(struct sockaddr_in);

	int iRet = -1;
	if(m_bBlocking) {
		// 阻塞
	}
	else {
		// 不阻塞
		struct timeval tout;
		tout.tv_sec = uiTimeout / 1000;
		tout.tv_usec = (uiTimeout % 1000) * 1000;
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(m_Socket, &rset);
		int iRetS = select(m_Socket + 1, &rset, NULL, NULL, &tout);
		if (iRetS == -1) {
			iRet = -1;
			goto EXIT_TCP_ACCEPT;
		} else if (iRetS == 0) {
			iRet = 0;
			goto EXIT_TCP_ACCEPT;
		}
	}

	accpetSocket = accept(m_Socket, (struct sockaddr *)&remoteAddr, (socklen_t *)&iRemoteAddrLen);

	if(accpetSocket != -1) {
		DLog("jni.DrTcpSocket::Accept()", "接收socket:(%d)成功", accpetSocket);
        
        // 设置心跳检测
        /*deal with the tcp keepalive
         iKeepAlive = 1 (check keepalive)
         iKeepIdle = 600 (active keepalive after socket has idled for 10 minutes)
         KeepInt = 60 (send keepalive every 1 minute after keepalive was actived)
         iKeepCount = 3 (send keepalive 3 times before disconnect from peer)
         */
        int iKeepAlive = 1, iKeepIdle = 15, KeepInt = 15, iKeepCount = 2;
        if (setsockopt(accpetSocket, SOL_SOCKET, SO_KEEPALIVE, (void*)&iKeepAlive, sizeof(iKeepAlive)) < 0) {
            iRet = -1;
            ELog("jni.DrTcpSocket::Accept()", "设置SO_KEEPALIVE失败！");
            goto EXIT_TCP_ACCEPT;
        }
        if (setsockopt(accpetSocket, IPPROTO_TCP, TCP_KEEPALIVE, (void *)&iKeepIdle, sizeof(iKeepIdle)) < 0) {
            iRet = -1;
            ELog("jni.DrTcpSocket::Accept()", "设置TCP_KEEPINTVL失败！");
            goto EXIT_TCP_ACCEPT;
        }
        if (setsockopt(accpetSocket, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&KeepInt, sizeof(KeepInt)) < 0) {
            iRet = -1;
            ELog("jni.DrTcpSocket::Accept()", "设置TCP_KEEPINTVL失败！");
            goto EXIT_TCP_ACCEPT;
        }
        if (setsockopt(accpetSocket, IPPROTO_TCP, TCP_KEEPCNT, (void *)&iKeepCount, sizeof(iKeepCount)) < 0) {
            iRet = -1;
            ELog("jni.DrTcpSocket::Accept()", "设置TCP_KEEPCNT失败！");
            goto EXIT_TCP_ACCEPT;
        }
        DLog("jni.DrTcpSocket::Accept()", "设置tcp心跳检测成功，每次%d秒检测一次，连续%d次没有收到心跳则断开，连接成功后%d秒开始心跳检测！", \
             KeepInt, iKeepCount, iKeepIdle);
        
        int iFlag = 1;
        // 不使用发送缓冲区
		setsockopt(accpetSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
		// 接收缓冲区大小
        iFlag = 1500;
		setsockopt(accpetSocket, SOL_SOCKET, SO_RCVBUF, (const char *)&iFlag, sizeof(iFlag));
		// socket可重用
		iFlag = 1;
		setsockopt(accpetSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&iFlag, sizeof(iFlag));
        
		clientSocket.setScoket(accpetSocket);
		clientSocket.SetAddress(remoteAddr);
		clientSocket.SetConnnected();
		clientSocket.setBlocking(bBlocking);

		iRet = 1;
	}

EXIT_TCP_ACCEPT:
	m_SocketMutex.unlock();
	if(iRet == -1) {
		Close();
	}
	return clientSocket;
}

void DrTcpSocket::Close() {
	DrSocket::Close();
	m_bConnected = false;
}
