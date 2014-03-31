/*
 * DrThread.cpp
 *
 *  Created on: 2014年2月14日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "DrThread.h"
#include <time.h>
#include "DrLog.h"
DrThread::DrThread() {
	m_pthread_t = 0;
	m_pthread_t_id = 0;
	m_isRunning = false;
	m_pDrRunnable = NULL;
}
DrThread::DrThread(DrRunnable *runnable) {
	m_pthread_t = 0;
	m_pthread_t_id = 0;
	m_isRunning = false;
	this->m_pDrRunnable = runnable;
}
DrThread::~DrThread() {
	stop();
}
void* DrThread::thread_proc_func(void *args){
	pthread_t threadId = 0;
	DrThread *pDrThread = (DrThread*)args;
	if(pDrThread){
		DLog("Jni.DrThread.thread_proc_func", "pDrThread:(%ld)->onRun", pDrThread->getThreadId());
		threadId = pDrThread->getThreadId();
		pDrThread->onRun();
	}
	DLog("Jni.DrThread.thread_proc_func", "线程:(%ld)已经运行完毕!", threadId);
	return (void*)0;
}
pthread_t DrThread::start(){
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	int ret = pthread_create(&m_pthread_t, &attrs, &thread_proc_func, (void*)this);
	if(0 != ret) {
		ELog("Jni.DrThread.start", "启动线程失败,因为:(%s)!", strerror(ret));
		m_pthread_t = 0;
		m_pthread_t_id = 0;
	}
	else {
		DLog("Jni.DrThread.start", "启动线程:(%ld)成功!", m_pthread_t);
		m_pthread_t_id = m_pthread_t;
	}
	return m_pthread_t;
}
void DrThread::stop() {
	if(isRunning()) {
		DLog("Jni.DrThread.stop", "等待线程:(%ld)退出...", m_pthread_t);
		if(0 != pthread_join(m_pthread_t, NULL)){
			DLog("Jni.DrThread.stop", "等待退出线程:(%ld)错误!", m_pthread_t);
		}
		else{
			DLog("Jni.DrThread.stop", "退出线程:(%ld)成功!", m_pthread_t);
		}
	}
	else if(m_pthread_t_id != 0){
		DLog("Jni.DrThread.stop", "线程:(%ld)已经退出!", m_pthread_t_id);
	}
	m_pthread_t = 0;
	m_pthread_t_id = 0;
}
void DrThread::sleep(uint32_t msec){
	//usleep(100);
//	struct timespect st;
//	st.tv_sec = msec / 1000;
//	st.tv_nsec = (msec % 1000) * 1000 * 1000;
//	if(-1 == nanosleep(&st, NULL)){
//		showLog("Jni.DrThread.sleep", "thread sleep error!");
//	}
}
pthread_t DrThread::getThreadId(){
	//return pthread_self();
	return m_pthread_t;
}
bool DrThread::isRunning(){
	bool bFlag = false;
	if(m_pthread_t > 0) {
		bFlag = true;
	}
	return bFlag;
}
void DrThread::onRun() {
	if(NULL != m_pDrRunnable ) {
		m_pDrRunnable->onRun();
	}
}
