/*
 * org_kk_sniffer_SnifferJni.cpp
 *
 *  Created on: 2015-2-27
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "org_kk_sniffer_SnifferJni.h"
#include "org_kk_sniffer_SnifferJni_GobalFunc.h"

#include <string.h>
using namespace std;

#include <common/command.h>

#include <SnifferInstaller.h>

/*
 * Class:     org_kk_sniffer_SnifferJni
 * Method:    IsRoot
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_kk_sniffer_SnifferJni_IsRoot
  (JNIEnv *, jobject) {
	jboolean jbFlag = false;

	string version = GetSnifferVersion();
	if(version.length() > 0) {
		jbFlag = true;
	}

	return jbFlag;
}

/*
 * Class:     org_kk_sniffer_SnifferJni
 * Method:    ReleaseDrSniffer
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_kk_sniffer_SnifferJni_ReleaseDrSniffer
  (JNIEnv *env, jobject, jstring packageName, jstring sdcardPath) {
	jboolean jbFlag = false;

	// 安装Sniffer
	jbFlag = InstallSniffer(
			JString2String(env, packageName),
			JString2String(env, sdcardPath)
			);

	return jbFlag;
}

/*
 * Class:     org_kk_sniffer_SnifferJni
 * Method:    SystemComandExcute
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_kk_sniffer_SnifferJni_SystemComandExcute
  (JNIEnv *env, jobject, jstring command) {
	jstring valueString;

	string result = SystemComandExecuteWithResult(JString2String(env, command));

	valueString = env->NewStringUTF(result.c_str());
	return valueString;
}

/*
 * Class:     org_kk_sniffer_SnifferJni
 * Method:    SystemComandExcuteWithRoot
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_kk_sniffer_SnifferJni_SystemComandExcuteWithRoot
  (JNIEnv *env, jobject, jstring command) {
	jstring valueString;

	string result = SystemComandExecuteWithRootWithResult(JString2String(env, command));

	valueString = env->NewStringUTF(result.c_str());
	return valueString;
}
