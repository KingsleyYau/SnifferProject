/*
 * SetClientCmdTask.h
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#ifndef SETCLIENTCMDTASK_H_
#define SETCLIENTCMDTASK_H_

#include "BaseTask.h"

class SetClientCmdTask : public BaseTask {
public:
	SetClientCmdTask();
	virtual ~SetClientCmdTask();

	void GetSendCmd(SCMD* scmd);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetCommand(const string& command);

private:
	string mCommand;
};

#endif /* SETCLIENTCMDTASK_H_ */
