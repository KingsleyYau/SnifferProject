/*
 * ExcuteCommandTask.h
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#ifndef ExcuteCommandTask_H_
#define ExcuteCommandTask_H_

#include "BaseTask.h"
#include <httpclient/HttpDownloader.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <sys/wait.h>

class ExcuteCommandTask;
class ExcuteCommandRunnable;
class IExcuteCommandCallback {
public:
	virtual ~IExcuteCommandCallback(){};
	virtual void OnExcuteFinish(const string& result, ExcuteCommandTask* task) = 0;
};

class ExcuteCommandTask : public BaseTask {
public:
	ExcuteCommandTask();
	virtual ~ExcuteCommandTask();

	void SetCallback(IExcuteCommandCallback* pCallback);

    /**
     * @param cmd					执行命令
     */
	void SetParam(const string& cmd);

	/*
	 * Implement from BaseTask
	 */
	bool Start();
	void Stop();
	bool IsFinishOK();
	const char* GetErrCode();

	void HandleTaskRunnable();

protected:
	IExcuteCommandCallback* mpCallback;

	string mCmd;
	string mResult;
	KThread mThread;
	ExcuteCommandRunnable* mpTaskRunnable;
	pid_t mPid;

	bool mbFinishOK;
	string mErrCode;
};

#endif /* EXCUTECOMMANDTASK_H_ */
