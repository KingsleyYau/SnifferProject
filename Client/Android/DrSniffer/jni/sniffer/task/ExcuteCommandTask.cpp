/*
 * ExcuteCommandTask.cpp
 *
 *  Created on: 2015-9-10
 *      Author: Max
 */

#include "ExcuteCommandTask.h"

/*
 * 处理线程
 */
class ExcuteCommandRunnable : public KRunnable {
public:
	ExcuteCommandRunnable(ExcuteCommandTask *pExcuteCommandTask) {
		mpExcuteCommandTask = pExcuteCommandTask;

	}

	virtual ~ExcuteCommandRunnable() {
		mpExcuteCommandTask = NULL;

	}

protected:
	void onRun() {
		if( mpExcuteCommandTask != NULL ) {
			mpExcuteCommandTask->HandleTaskRunnable();
		}
	}
private:
	ExcuteCommandTask *mpExcuteCommandTask;
};

ExcuteCommandTask::ExcuteCommandTask() {
	// TODO Auto-generated constructor stub
	mCmd = "";
	mpCallback = NULL;
	mPid = -1;
	mpTaskRunnable = new ExcuteCommandRunnable(this);
}

ExcuteCommandTask::~ExcuteCommandTask() {
	// TODO Auto-generated destructor stub
}

void ExcuteCommandTask::SetCallback(IExcuteCommandCallback* pCallback) {
	mpCallback = pCallback;
}

void ExcuteCommandTask::SetParam(
		const string& cmd
		) {

	mCmd = cmd;
}

bool ExcuteCommandTask::Start() {
	bool bFlag = false;

	if( BaseTask::Start() ) {
		if( mThread.start(mpTaskRunnable) != -1 ) {
			bFlag = true;
		} else {
			OnTaskFinish();
		}
	}

	FileLog(
			SnifferLogFileName,
			"ExcuteCommandTask::Start( "
			"bFlag : %s "
			")",
			bFlag?"true":"false"
			);

	return bFlag;
}

void ExcuteCommandTask::Stop() {
	if( mPid > 0 ) {
		kill(mPid, 9);
		mPid = -1;
	}

	mThread.stop();
	BaseTask::Stop();
	BaseTask::Reset();
}

bool ExcuteCommandTask::IsFinishOK() {
	return mbFinishOK;
}

const char* ExcuteCommandTask::GetErrCode() {
	return mErrCode.c_str();
}

void ExcuteCommandTask::HandleTaskRunnable() {
	char buffer[1024];
	int cgi_pipe[2];
	int ret = 0;

	if( (mPid = fork()) < 0 ) {
		// fail
		OnTaskFinish();

	} else if (mPid == 0) {
		// child process
		// close parent pipe
		close(cgi_pipe[0]);

		// dup2 child stand output to pipe
		dup2(cgi_pipe[1], STDOUT_FILENO);

		// dup2 child stand input to pipe
		dup2(cgi_pipe[1], STDIN_FILENO);

		execl(mCmd.c_str(), mCmd.c_str(), NULL);

		exit(0);

	} else {
		// parent process
		// close child pipe
		close(cgi_pipe[1]);

		// read from pipe
		while( (ret = read(cgi_pipe[0], buffer, sizeof(buffer) - 1)) > 0 ) {
			buffer[ret] = '\0';
			mResult += buffer;
		}

		int status;
		waitpid(mPid, &status, 0);

		if( mpCallback != NULL ) {
			mpCallback->OnExcuteFinish(mResult, this);
		}
	}

}
