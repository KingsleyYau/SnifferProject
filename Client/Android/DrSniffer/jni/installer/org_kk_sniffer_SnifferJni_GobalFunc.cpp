/*
 * org_kk_sniffer_SnifferJni_GobalFunc.cpp
 *
 *  Created on: 2015-3-4
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "org_kk_sniffer_SnifferJni_GobalFunc.h"

JavaVM* gJavaVM;

string JString2String(JNIEnv* env, jstring str) {
	string result("");
	if (NULL != str) {
		const char* cpTemp = env->GetStringUTFChars(str, 0);
		result = cpTemp;
		env->ReleaseStringUTFChars(str, cpTemp);
	}
	return result;
}

/* JNI_OnLoad */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	FileLog("installer", "JNI_OnLoad( JNI.so JNI_OnLoad )");
	gJavaVM = vm;

	// Get JNI
	JNIEnv* env;
	if (JNI_OK != vm->GetEnv(reinterpret_cast<void**> (&env),
                           JNI_VERSION_1_4)) {
		FileLog("installer", "JNI_OnLoad ( JNI.so could not get JNI env )");
		return -1;
	}

	KLog::SetLogDirectory("/sdcard/installer/");

	return JNI_VERSION_1_4;
}

bool GetEnv(JNIEnv** env, bool* isAttachThread) {
	*isAttachThread = false;
	jint iRet = JNI_ERR;
	iRet = gJavaVM->GetEnv((void**) env, JNI_VERSION_1_4);
	if( iRet == JNI_EDETACHED ) {
		iRet = gJavaVM->AttachCurrentThread(env, NULL);
		*isAttachThread = (iRet == JNI_OK);
	}

	return (iRet == JNI_OK);
}

bool ReleaseEnv(bool isAttachThread) {
	if (isAttachThread) {
		gJavaVM->DetachCurrentThread();
	}
	return true;
}
