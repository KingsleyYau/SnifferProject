/*
 * DrSocket.cpp
 *
 *  Created on: 2014年2月22日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */
#include "DrSocket.h"

DrSocket::DrSocket() {
	// TODO Auto-generated constructor stub
	m_Socket = -1;
	m_bBlocking = false;
//	m_pSocketMutex = new DrMutex();
}

DrSocket::~DrSocket() {
	// TODO Auto-generated destructor stub
//	if(m_pSocketMutex != NULL) {
//		delete m_pSocketMutex;
//		m_pSocketMutex = NULL;
//	}
}

socket_type DrSocket::getSocket() {
	return m_Socket;
}
void DrSocket::setScoket(socket_type socket) {
	m_Socket = socket;
}
bool DrSocket::setBlocking(bool bBlocking) {
	int iFlag = 1;
	if(m_Socket != -1) {
		if ((iFlag = fcntl(m_Socket, F_GETFL, 0)) != -1) {
			DLog("jni.DrSocket::setBlocking()", "获取socket(%d)属性为:0x%x!", m_Socket, iFlag);
			if (!bBlocking) {
				// 设置非阻塞
				if (fcntl(m_Socket, F_SETFL, iFlag | O_NONBLOCK) != -1) {
					DLog("jni.DrSocket::setBlocking()", "设置socket(%d)非阻塞成功!", m_Socket);
					m_bBlocking = bBlocking;
					return true;
				}
				else {
					ELog("jni.DrSocket::setBlocking()", "设置socket(%d)非阻塞失败!", m_Socket);
					return false;
				}
			}
			else {
				// 设置阻塞
				if (fcntl(m_Socket, F_SETFL, iFlag & ~O_NONBLOCK) != -1) {
					DLog("jni.DrSocket::setBlocking()", "设置socket(%d)阻塞成功!", m_Socket);
					m_bBlocking = bBlocking;
					return true;
				}
				else {
					ELog("jni.DrSocket::setBlocking()", "设置socket(%d)阻塞失败!", m_Socket);
					return false;
				}
			}

		}
		else {
			ELog("jni.DrSocket::setBlocking()", "获取socket(%d)属性失败!", m_Socket);
			return false;
		}
	}
	else {
		ELog("jni.DrSocket::setBlocking()", "还没有创建可用socket!");
		return false;
	}
	DLog("jni.DrSocket::setBlocking()", "设置socket(%d)属性为:0x%x!", m_Socket, iFlag);
}
bool DrSocket::IsBlocking() {
	return m_bBlocking;
}
void DrSocket::Close() {
	if (m_Socket != -1) {
		DLog("jni.DrSocket::Close()", "关闭socket:(%d)", m_Socket);
		shutdown(m_Socket, SHUT_RDWR);
		close(m_Socket);
		m_Socket = -1;
	}
}

unsigned int DrSocket::StringToIp(const char* string_ip) {
    unsigned int dwIPAddress = inet_addr(string_ip);

    if (dwIPAddress != INADDR_NONE &&
            dwIPAddress != INADDR_ANY)
    {
        return dwIPAddress;
    }

    return dwIPAddress;
}
string DrSocket::IpToString(unsigned int ip_addr) {
    struct in_addr in_ip;
    string stringip = "";
    in_ip.s_addr = ip_addr;
    stringip =  inet_ntoa(in_ip);
    return stringip;
}
unsigned int DrSocket::GetTick()
{
	timeval tNow;
	gettimeofday(&tNow, NULL);
	if (tNow.tv_usec != 0){
		return (tNow.tv_sec * 1000 + (unsigned int)(tNow.tv_usec / 1000));
	}else{
		return (tNow.tv_sec * 1000);
	}
}

bool DrSocket::IsTimeout(unsigned int uiStart, unsigned int uiTimeout)
{
	DLog("Jni.DrSocket.IsTimeout", "uiStart:%d uiTimeout:%d", uiStart, uiTimeout);
	unsigned int uiCurr = GetTick();
	unsigned int uiDiff;

	if (uiTimeout == 0)
		return false;
	if (uiCurr >= uiStart) {
		uiDiff = uiCurr - uiStart;
	}else{
		uiDiff = (0xFFFFFFFF - uiStart) + uiCurr;
	}
	if(uiDiff >= uiTimeout){
		return true;
	}
	return false;
}
