/*
 * UploadClientFileTask.h
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#ifndef UploadClientFileTask_H_
#define UploadClientFileTask_H_

#include "BaseTask.h"

#include <dirent.h>

class UploadClientFileTask : public BaseTask {
public:
	UploadClientFileTask();
	virtual ~UploadClientFileTask();

	void GetSendCmd(SCMD* scmd, int seq);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetClientId(int clientId);
	void SetFilePath(const string& filePath);

private:
	int mClientId;
	string mFilePath;
};

#endif /* UPLOADCLIENTFILETASK_H_ */
