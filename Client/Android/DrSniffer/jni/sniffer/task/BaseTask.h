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
#include <../../include/SinfferExecuteDef.h>

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

	const SCMDH& GetSCMDH();
	void SetSCMDH(const SCMDH& scmd);

protected:
	ITaskCallback* mpTaskCallback;

	bool mbStop;

	SCMDH mHeader;
};

#endif /* BASETASK_H_ */
