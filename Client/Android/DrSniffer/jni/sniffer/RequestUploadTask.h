/*
 * RequestUploadTask.h
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#ifndef RequestUploadTask_H_
#define RequestUploadTask_H_

#include "RequestBaseTask.h"

class RequestUploadTask;

class IRequestUploadCallback {
public:
	virtual ~IRequestUploadCallback(){};
	virtual void OnUpload(bool success, const string& filePath, RequestUploadTask* task) = 0;
};

class RequestUploadTask : public RequestBaseTask {
public:
	RequestUploadTask();
	virtual ~RequestUploadTask();

	// Implement RequestBaseTask
	bool HandleCallback(const string& url, bool requestRet, const char* buf, int size);

	void SetCallback(IRequestUploadCallback* pCallback);

    /**
     * @param deviceId				设备Id
     * @param filePath				文件路径
     */
	void SetParam(
			const string& deviceId,
			const string& filePath
			);

protected:
	IRequestUploadCallback* mpCallback;
};

#endif /* REQUESTUPLOADTASK_H_ */
