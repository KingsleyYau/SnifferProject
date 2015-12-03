/*
 * BaseTask.h
 *
 *  Created on: 2015-12-3
 *      Author: Max
 */

#ifndef BASETASK_H_
#define BASETASK_H_

#include "ITask.h"

#include <json/json.h>

#include <common/Buffer.h>
#include <common/StringHandle.h>

class BaseTask : public ITask {
public:
	BaseTask();
	virtual ~BaseTask();

	PROTOCOLTYPE GetPtType();
	void SetPtType(PROTOCOLTYPE ptType);

protected:
	PROTOCOLTYPE mPtType;
};

#endif /* BASETASK_H_ */
