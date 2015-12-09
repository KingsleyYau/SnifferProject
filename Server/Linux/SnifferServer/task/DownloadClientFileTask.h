/*
 * DownloadClientFileTask.h
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#ifndef DownloadClientFileTask_H_
#define DownloadClientFileTask_H_

#include "BaseTask.h"

#include <dirent.h>

class DownloadClientFileTask : public BaseTask {
public:
	DownloadClientFileTask();
	virtual ~DownloadClientFileTask();

	void GetSendCmd(SCMD* scmd, int seq);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetClientId(int clientId);
	void SetUrl(const string& url);
	void SetFilePath(const string& filePath);

private:
	int mClientId;
	string mUrl;
	string mFilePath;
};

#endif /* DOWNLOADCLIENTFILETASK_H_ */
