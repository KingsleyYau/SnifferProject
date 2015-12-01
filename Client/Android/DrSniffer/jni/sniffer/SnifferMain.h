/*
 * SnifferMain.h
 *
 *  Created on: 2015-11-14
 *      Author: Max
 */

#ifndef SNIFFERMAIN_H_
#define SNIFFERMAIN_H_

#include "Sniffer.h"
#include "SnifferClient.h"
#include "RequestUploadTask.h"

#include <httpclient/HttpRequestHostManager.h>
#include <httpclient/HttpRequestManager.h>

#include <common/command.h>
#include <json/json/json.h>

class SnifferMain : public SnifferClientCallback, public ITaskCallback {
public:
	SnifferMain();
	virtual ~SnifferMain();

	/**
	 * Implement from SnifferClientCallback
	 */
	void OnRecvCommand(SnifferClient* client, const SCMD &scmd);

	bool Run();

private:
	/**
	 * Implement from ITaskCallback
	 */
	void OnTaskFinish(ITask* pTask);

	Sniffer mSniffer;
	SnifferClient mSnifferClient;

	HttpRequestHostManager mHttpRequestHostManager;
	HttpRequestManager mHttpRequestManager;
};

#endif /* SNIFFERMAIN_H_ */
