/*
 * BaseTask.h
 *
 *  Created on: 2015-9-15
 *      Author: Max
 */

#ifndef BASETASK_H_
#define BASETASK_H_

#include <string>
using namespace std;

#include <common/KLog.h>

#include "ITask.h"

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

protected:
	ITaskCallback* mpTaskCallback;

	bool mbStop;
};

#endif /* BASETASK_H_ */
