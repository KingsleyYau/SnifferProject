/*
 * GetClientDirTask.h
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#ifndef GETCLIENTDIRTASK_H_
#define GETCLIENTDIRTASK_H_

#include "ITask.h"

#include <string>
using namespace std;

class GetClientDirTask {
public:
	GetClientDirTask();
	virtual ~GetClientDirTask();

	void GetSendCmd(SCMD* scmd);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetDir(const char* dir);

private:
	char mDir[1024];
};

#endif /* GETCLIENTDIRTASK_H_ */
