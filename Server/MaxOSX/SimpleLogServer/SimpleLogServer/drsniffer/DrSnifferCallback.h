/*
 * File         : DrSnifferCallback.h
 * Date         : 2012-07-02
 * Author       : Kingsley Yau
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrPalm DrJniCallback include
 */
#ifndef _INC_DRJNICALLBACK_
#define _INC_DRJNICALLBACK_
typedef void (*OnNewClientOnline)(DrTcpSocket aSocket);
typedef void (*OnRecvCommand)(DrTcpSocket aSocket, SSCMD sscmd);

typedef struct _DrSnifferCallback{
    OnNewClientOnline onNewClientOnline;
	OnRecvCommand onRecvCommand;
}DrSnifferCallback;
#endif
