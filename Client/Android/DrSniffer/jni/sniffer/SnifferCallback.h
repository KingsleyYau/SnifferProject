/*
 * File         : SnifferCallback.h
 * Date         : 2012-07-02
 * Author       : Kingsley Yau
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : JniCallback include
 */
#ifndef _INCJNICALLBACK_
#define _INCJNICALLBACK_
typedef void (*OnNewClientOnline)(KTcpSocket aSocket);
typedef void (*OnRecvCommand)(KTcpSocket aSocket, SSCMD sscmd);

typedef struct SnifferCallback {
    OnNewClientOnline onNewClientOnline;
	OnRecvCommand onRecvCommand;
} SnifferCallback;
#endif
