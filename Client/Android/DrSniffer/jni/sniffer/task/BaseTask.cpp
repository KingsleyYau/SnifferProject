/*
 * BaseTask.cpp
 *
 *  Created on: 2015-9-15
 *      Author: Max
 */

#include "BaseTask.h"

BaseTask::BaseTask() {
	// TODO Auto-generated constructor stub
	mpTaskCallback = NULL;
	mbStop = false;
}

BaseTask::~BaseTask() {
	// TODO Auto-generated destructor stub
}

void BaseTask::SetTaskCallback(ITaskCallback* pTaskCallback) {
	mpTaskCallback = pTaskCallback;
}

ITaskCallback* BaseTask::GetTaskCallback() {
	return mpTaskCallback;
}

bool BaseTask::Start() {
	FileLog(SnifferLogFileName, "BaseTask::Start( this : %p, mbStop : %s )", this, mbStop?"true":"false");
	bool bFlag = !mbStop;
	if( !bFlag ) {
		OnTaskFinish();
	}
	return bFlag;
}

void BaseTask::Stop() {
	FileLog(SnifferLogFileName, "BaseTask::Stop( this : %p )", this);
	mbStop = true;
}

void BaseTask::Reset() {
	mbStop = false;
}

void BaseTask::OnTaskFinish() {
	FileLog(SnifferLogFileName, "BaseTask::OnTaskFinish( this : %p )", this);
	if( mpTaskCallback != NULL ) {
		mpTaskCallback->OnTaskFinish(this);
	}
}

int BaseTask::GetSeq() {
	return mSeq;
}

void BaseTask::SetSeq(int seq) {
	mSeq = seq;
}
