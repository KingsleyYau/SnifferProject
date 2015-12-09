/*
 * GetClientDirTask.h
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#ifndef GETCLIENTDIRTASK_H_
#define GETCLIENTDIRTASK_H_

#include "BaseTask.h"

#include <dirent.h>

class GetClientDirTask : public BaseTask {
public:
	GetClientDirTask();
	virtual ~GetClientDirTask();

	void GetSendCmd(SCMD* scmd, int seq);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetDir(const string& dir);
	void SetClientId(int clientId);
	void SetPageIndex(int pageIndex);
	void SetPageSize(int pageSize);

private:
	int mClientId;
	string mDir;
	int mPageIndex;
	int mPageSize;
};

#endif /* GETCLIENTDIRTASK_H_ */
