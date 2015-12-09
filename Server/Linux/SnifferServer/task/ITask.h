/*
 * ITask.h
 *
 *  Created on: 2015-11-14
 *      Author: Max
 */

#ifndef ITASK_H_
#define ITASK_H_

#include <SnifferCommandDef.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
using namespace std;

class ITask {
public:
	virtual ~ITask(){};

	virtual void GetSendCmd(SCMD* scmd, int seq) = 0;
	virtual bool GetReturnData(SCMD* scmd, char* buffer, int& len) = 0;
};

#endif /* ITASK_H_ */
