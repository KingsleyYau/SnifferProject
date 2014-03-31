/* Header for class com_drcom_drsniffer_DrSnifferJni
 * Created on: 2014年3月11日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */
#include "com_drcom_drsniffer_DrSnifferJni.h"

#include <unistd.h>
#include <string>
#include <stl/_string.h>

#include "common/command.h"
#include "drsniffer/DrSnifferInstaller.h"
#include "drsniffer/DrSnifferServer.h"

DrSnifferServer g_DrSnifferServer;

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    IsRoot
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_drsniffer_DrSnifferJni_IsRoot
  (JNIEnv *env, jobject thiz) {
	jboolean jbFlag = false;

	string version = GetDrSnifferVersion();
	if(version.length() > 0) {
		jbFlag = true;
	}

	return jbFlag;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    ReleaseDrSniffer
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_drsniffer_DrSnifferJni_ReleaseDrSniffer
  (JNIEnv *env, jobject thiz, jstring sdcardPath) {
	jboolean jbFlag = false;

	const char *pPath = env->GetStringUTFChars(sdcardPath, 0);
	string filePath = pPath;
	env->ReleaseStringUTFChars(sdcardPath, pPath);

	// 安装DrSniffer
	jbFlag = InstallDrSniffer(filePath);

	return jbFlag;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    SystemComandExcute
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_drsniffer_DrSnifferJni_SystemComandExcute
  (JNIEnv *env, jobject thiz, jstring command) {
	jstring valueString;

	const char *pCommand = env->GetStringUTFChars(command, 0);
	string result = SystemComandExecuteWithResult(pCommand);
	env->ReleaseStringUTFChars(command, pCommand);

	valueString = env->NewStringUTF(result.c_str());
	return valueString;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    SystemComandExcuteWithRoot
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_drsniffer_DrSnifferJni_SystemComandExcuteWithRoot
  (JNIEnv *env, jobject thiz, jstring command) {
	jstring valueString;

	const char *pCommand = env->GetStringUTFChars(command, 0);
	string result = SystemComandExecuteWithRootWithResult(pCommand);
	env->ReleaseStringUTFChars(command, pCommand);

	valueString = env->NewStringUTF(result.c_str());
	return valueString;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    StartServer
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_drsniffer_DrSnifferJni_StartServer
	(JNIEnv *env, jobject thiz) {
	jboolean jbFlag = false;
	jbFlag = g_DrSnifferServer.Start(256, ServerPort);
	return jbFlag;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    StopServer
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_drsniffer_DrSnifferJni_StopServer
	(JNIEnv *env, jobject thiz)  {
	jboolean jbFlag = false;
	g_DrSnifferServer.Stop();
	return jbFlag;
}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    GetOnlineClientList
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_drcom_drsniffer_DrSnifferJni_GetOnlineClientList
	(JNIEnv *env, jobject thiz)  {

}

/*
 * Class:     com_drcom_drsniffer_DrSnifferJni
 * Method:    SendCommandToClient
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_drcom_drsniffer_DrSnifferJni_SendCommandToClient
	(JNIEnv *env, jobject thiz) {
}
