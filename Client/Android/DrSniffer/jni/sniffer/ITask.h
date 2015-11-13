/*
 * ITask.h
 *
 *  Created on: 2015-9-11
 *      Author: Max
 */

#ifndef ITASK_H_
#define ITASK_H_

class ITask {
public:
	virtual ~ITask(){};
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsFinishOK() = 0;
	virtual const char* GetErrCode() = 0;
};

class ITaskCallback {
public:
	virtual ~ITaskCallback(){};
	virtual void OnTaskFinish(ITask* pTask) = 0;
};
#endif /* ITASK_H_ */
