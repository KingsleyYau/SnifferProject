/*
 * UpdateClientTask.h
 *
 *  Created on: 2015年11月15日
 *      Author: kingsley
 */

#ifndef UpdateClientTask_H_
#define UpdateClientTask_H_

#include "BaseTask.h"

class UpdateClientTask : public BaseTask {
public:
	UpdateClientTask();
	virtual ~UpdateClientTask();

	void GetSendCmd(SCMD* scmd, int seq);
	bool GetReturnData(SCMD* scmd, char* buffer, int& len);

	void SetUrl(const string& url);
	void SetVersion(const string& version);

private:
	string mUrl;
	string mVersion;
};

#endif /* UPDATECLIENTTASK_H_ */
