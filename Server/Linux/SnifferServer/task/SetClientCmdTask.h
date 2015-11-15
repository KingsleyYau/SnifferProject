/*
 * SetClientCmdTask.h
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#ifndef SETCLIENTCMDTASK_H_
#define SETCLIENTCMDTASK_H_

#include "ITask.h"

class SetClientCmdTask : public ITask {
public:
	SetClientCmdTask();
	virtual ~SetClientCmdTask();

	void GetSendCmd(SCMD* scmd);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetCommand(const char* command);

private:
	char mCommand[1024];
};

#endif /* SETCLIENTCMDTASK_H_ */
