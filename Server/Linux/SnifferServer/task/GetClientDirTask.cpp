/*
 * GetClientDirTask.cpp
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#include "GetClientDirTask.h"

GetClientDirTask::GetClientDirTask() {
	// TODO Auto-generated constructor stub

}

GetClientDirTask::~GetClientDirTask() {
	// TODO Auto-generated destructor stub
}

void GetClientDirTask::GetSendCmd(SCMD* scmd) {
	string command = "ls -al";
	if( strlen(mDir) > 0 ) {
		command += " ";
		command += mDir;
	}

	scmd->header.scmdt = ExcuteCommand;
	scmd->header.bNew = true;
	scmd->header.len = command.length();
	memcpy(scmd->param, command.c_str(), scmd->header.len);

}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
	sprintf(buffer,
			"<html>\n<body>\n"
			"%s"
			"</body>\n</html>\n",
			scmd->param
			);
	return true;
//	param = "<html>\n<body>\n";
//	string herf = " <a href=\"";
//	herf += GET_CLIENT_DIR;
//	herf += "?";
//	herf += CLIENT_ID;
//	herf += "=";
//	herf += client->fd;
//	herf += DIRECTORY;
//	herf += "=/data";
//	herf += "\">upload</a></br>\n";
//	param += StringHandle::replace(scmd->param, "\n", herf);
//	param += "</body>\n</html>\n";
}

void GetClientDirTask::SetDir(const char* dir) {
	snprintf(mDir, 1023, "%s", dir);
}
