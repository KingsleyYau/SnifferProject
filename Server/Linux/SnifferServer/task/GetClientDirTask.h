/*
 * GetClientDirTask.h
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#ifndef GETCLIENTDIRTASK_H_
#define GETCLIENTDIRTASK_H_

#include "ITask.h"

#include <dirent.h>
#include <json/json.h>
#include <common/Buffer.h>
#include <common/StringHandle.h>

class GetClientDirTask : public ITask {
public:
	GetClientDirTask();
	virtual ~GetClientDirTask();

	void GetSendCmd(SCMD* scmd);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetDir(const char* dir);
	void SetClientId(int clientId);
	void SetPageIndex(int pageIndex);
	void SetPageSize(int pageSize);

private:
	string mDir;
	int mClientId;
	int mPageIndex;
	int mPageSize;
};

#endif /* GETCLIENTDIRTASK_H_ */
