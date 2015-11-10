/*
 * ORG_KK_SNIFFER_SNIFFERJNI_GOBALFUNC.h
 *
 *  Created on: 2015-3-4
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef ORG_KK_SNIFFER_SNIFFERJNI_GOBALFUNC_H_
#define ORG_KK_SNIFFER_SNIFFERJNI_GOBALFUNC_H_

#include <jni.h>

#include <string>
using namespace std;

#include <common/KLog.h>

extern JavaVM* gJavaVM;

string JString2String(JNIEnv* env, jstring str);

bool GetEnv(JNIEnv** env, bool* isAttachThread);
bool ReleaseEnv(bool isAttachThread);

#endif /* ORG_KK_SNIFFER_SNIFFERJNI_GOBALFUNC_H_ */
