/*
 * GetClientDirTask.cpp
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#include "GetClientDirTask.h"

GetClientDirTask::GetClientDirTask() {
	// TODO Auto-generated constructor stub
	mDir = "/";
}

GetClientDirTask::~GetClientDirTask() {
	// TODO Auto-generated destructor stub
}

void GetClientDirTask::GetSendCmd(SCMD* scmd) {
	printf("# GetClientDirTask::GetSendCmd(SCMD* scmd) \n");
	string command = "ls";
	command += " ";
	command += mDir;
	printf("# GetClientDirTask::GetSendCmd( command : %s ) \n", command.c_str());
	scmd->header.scmdt = ExcuteCommand;
	scmd->header.bNew = true;
	scmd->header.len = command.length();
	memcpy(scmd->param, command.c_str(), scmd->header.len);

}

bool GetClientDirTask::GetReturnData(SCMD* scmd, char* buffer, int& len) {
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
	if( dir != NULL ) {
		mDir = dir;
	}
}
