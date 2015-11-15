/*
 * SetClientCmdTask.cpp
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#include "SetClientCmdTask.h"

SetClientCmdTask::SetClientCmdTask() {
	// TODO Auto-generated constructor stub
	mCommand = {'\0'};
}

SetClientCmdTask::~SetClientCmdTask() {
	// TODO Auto-generated destructor stub
}

void SetClientCmdTask::GetSendCmd(SCMD* scmd) {
	scmd->header.scmdt = ExcuteCommand;
	scmd->header.bNew = true;
	scmd->header.len = strlen(mCommand);
	memcpy(scmd->param, mCommand, scmd->header.len);
}

bool SetClientCmdTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	bool bFalg = false;
	if( buffer != NULL ) {
		sprintf(buffer,
				"<html>\n<body>\n"
				"%s"
				"</body>\n</html>\n",
				scmd->param
				);
		len = strlen(scmd->param);
		bFalg = true;
	}

	return bFalg;
}

void SetClientCmdTask::SetCommand(const char* command) {
	snprintf(mCommand, 1023, "%s", command);
}
