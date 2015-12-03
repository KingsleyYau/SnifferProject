/*
 * BaseTask.cpp
 *
 *  Created on: 2015-12-3
 *      Author: Max
 */

#include "BaseTask.h"

BaseTask::BaseTask() {
	// TODO Auto-generated constructor stub
	mPtType = HTML;
}

BaseTask::~BaseTask() {
	// TODO Auto-generated destructor stub
}

PROTOCOLTYPE BaseTask::GetPtType() {
	return mPtType;
}

void BaseTask::SetPtType(PROTOCOLTYPE ptType) {
	mPtType = ptType;
}
