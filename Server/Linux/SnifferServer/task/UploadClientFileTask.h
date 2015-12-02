/*
 * UploadClientFileTask.h
 *
 *  Created on: 2015-11-25
 *      Author: Max
 */

#ifndef UploadClientFileTask_H_
#define UploadClientFileTask_H_

#include "ITask.h"

#include <dirent.h>
#include <json/json.h>
#include <common/Buffer.h>
#include <common/StringHandle.h>

class UploadClientFileTask : public ITask {
public:
	UploadClientFileTask();
	virtual ~UploadClientFileTask();

	void GetSendCmd(SCMD* scmd);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetFilePath(const char* filePath);
	void SetClientId(int clientId);

private:
	int mClientId;
	string mFilePath;
};

#endif /* UploadClientFileTask_H_ */
