/*
 * SetClientCmdTask.cpp
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#include "SetClientCmdTask.h"

SetClientCmdTask::SetClientCmdTask() {
	// TODO Auto-generated constructor stub
	mCommand = "";
}

SetClientCmdTask::~SetClientCmdTask() {
	// TODO Auto-generated destructor stub
}

void SetClientCmdTask::GetSendCmd(SCMD* scmd) {
	scmd->header.scmdt = ExcuteCommand;
	scmd->header.bNew = true;
	scmd->header.len = mCommand.length();
	memcpy(scmd->param, mCommand.c_str(), scmd->header.len);
}

bool SetClientCmdTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFalg = false;
	if( buffer != NULL ) {
		snprintf(buffer,
				MAXLEN - 1,
				"<html>\n<body>\n"
				"%s"
				"</body>\n</html>\n",
				StringHandle::replace(scmd->param, "\n", "\n</br>").c_str()
				);
		len = strlen(buffer);
		bFalg = true;
	}

	return bFalg;
}

void SetClientCmdTask::SetCommand(const char* command) {
	if( command != NULL ) {
		mCommand = command;
	}
}
