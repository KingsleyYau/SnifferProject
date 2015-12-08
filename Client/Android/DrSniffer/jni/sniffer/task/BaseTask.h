/*
 * BaseTask.h
 *
 *  Created on: 2015-9-15
 *      Author: Max
 */

#ifndef BASETASK_H_
#define BASETASK_H_

#include "ITask.h"
#include <SnifferCommandDef.h>
#include <SinfferExecuteDef.h>

#include <common/KLog.h>

#include <string>
using namespace std;

class BaseTask : public ITask {
public:
	BaseTask();
	virtual ~BaseTask();

	void SetTaskCallback(ITaskCallback* pTaskCallback);
	ITaskCallback* GetTaskCallback();

	// Implement from ITask
	virtual bool Start();
	virtual void Stop();
	virtual void Reset();

	virtual void OnTaskFinish();

	int GetSeq();
	void SetSeq(int seq);

protected:
	ITaskCallback* mpTaskCallback;

	bool mbStop;

	int mSeq;
};

#endif /* BASETASK_H_ */
