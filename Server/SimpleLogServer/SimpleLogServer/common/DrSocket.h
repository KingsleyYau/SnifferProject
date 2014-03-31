/*
 * DrSocket.h
 *
 *  Created on: 2014年2月22日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 *  Description: Thread safe socket class
 */

#ifndef DRSOCKET_H_
#define DRSOCKET_H_

#include <string>
using namespace std;

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <resolv.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>


#include "DrLog.h"
#include "DrMutex.h"

typedef int socket_type;

class DrSocket {
public:
	DrSocket();
	virtual ~DrSocket();

	virtual void Close();

	socket_type getSocket();
	void setScoket(socket_type socket);

	bool setBlocking(bool bBlocking);
	bool IsBlocking();

	static unsigned int StringToIp(const char* string_ip);
	static string IpToString(unsigned int ip_addr);

protected:
	unsigned int GetTick();
	bool IsTimeout(unsigned int uiStart, unsigned int uiTimeout);

	socket_type m_Socket;

	// 是否阻塞
	bool m_bBlocking;
//	 定义为指针是因为在ndk中不new而用成员变量可能会挂，原因不明
//	DrMutex* m_pSocketMutex;
	DrMutex m_SocketMutex;
};

#endif /* DRSOCKET_H_ */
