/*
 * DrLog.h
 *
 *  Created on: 2014年2月14日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef _INC_DRLOG_
#define _INC_DRLOG_

#include <string>
//#include <stl/_list.h> // just for ide error
using namespace std;

#include <dirent.h>
#include <sys/stat.h>

#define PRINT_JNI_LOG
#ifdef PRINT_JNI_LOG
//#include <android/log.h>
#define showLog(tag, format, ...) printf(format, ## __VA_ARGS__);printf("\r\n")
#define DLog(tag, format, ...) printf(format, ## __VA_ARGS__);printf("\r\n")
#define ILog(tag, format, ...) printf(format, ## __VA_ARGS__);printf("\r\n")
#define ELog(tag, format, ...) printf(format, ## __VA_ARGS__);printf("\r\n")
#define PLog(tag, format, ...) printf(format, ## __VA_ARGS__);printf("\r\n")
#define FileLog(fileNamePre, format, ...) ILog("jni", format,  ## __VA_ARGS__); LogToFile(fileNamePre, 1, format,  ## __VA_ARGS__)
#else
#define showLog(tag, format, ...)
#define DLog(tag, format, ...)
#define ILog(tag, format, ...)
#define ELog(tag, format, ...)
#define PLog(tag, format, ...)
#define FileLog(fileNamePre, format, ...)
#endif /* PRINT_JNI_LOG */

static inline int LogToFile(const char *fileNamePre, int level, const char *fmt, ...) {
	char pTimeBuffer[1024] = {'\0'}, pDataBuffer[4096] = {'\0'};

	// 输出当前时间
	time_t stm = time(NULL);
	struct tm tTime;
	localtime_r(&stm, &tTime);
	snprintf(pTimeBuffer, 64, "[ %d-%02d-%02d %02d:%02d:%02d ]", \
			tTime.tm_year+1900, tTime.tm_mon+1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec);

	// 输出日志数据
    va_list	agList;
    va_start(agList, fmt);
    vsnprintf(pDataBuffer, sizeof(pDataBuffer), fmt, agList);
    va_end(agList);

    strcat(pDataBuffer, "\n");

    // 生成日志文件目录
    string sLogFileDir = "/system/log";
    string sLogFileName = fileNamePre;

    DIR* dir = opendir(sLogFileDir.c_str());
	if (dir == NULL) {
		int mod = S_IRWXU | S_IRWXG | S_IRWXO;
		if( 0 == mkdir(sLogFileDir.c_str(), mod) ) {
			//DLog("jni.DrLog.LogToFile", "创建目录%s成功!", sLogFileDir.c_str());
		}
		else {
			//ELog("jni.DrLog.LogToFile", "创建目录%s失败!", sLogFileDir.c_str());
		}
	} else {
		closedir(dir);
	}

	// 生成日志文件
    char pLogFilePath[1024] = {'\0'};
    snprintf(pLogFilePath, sizeof(pLogFilePath), "%s/%s%d%02d%02d%02d.txt", \
    		sLogFileDir.c_str(), sLogFileName.c_str(), tTime.tm_year+1900, tTime.tm_mon+1, \
    		tTime.tm_mday, tTime.tm_hour/*, tTime.tm_min, tTime.tm_sec*/);

    FILE* file = fopen(pLogFilePath, "a+b");
    if(file != NULL) {
    	//DLog("jni.DrLog.LogToFile", "创建日志文件%s成功!", pLogFilePath);
    	int iLen = -1;
    	fseek(file, 0L, SEEK_END);
    	// 写入日志内容
    	if(pTimeBuffer) {
    		iLen = fwrite(pTimeBuffer, 1, strlen(pTimeBuffer), file);
    		if(iLen != -1 ) {
    			//DLog("jni.DrLog.LogToFile", "写入内容:%s成功!", pTimeBuffer);
    		}
    		else {
    			//ELog("jni.DrLog.LogToFile", "写入内容:%s失败!", pTimeBuffer);
    		}
    	}
    	if(pDataBuffer) {
    		iLen = fwrite(pDataBuffer, 1, strlen(pDataBuffer), file);
    		if(iLen != -1 ) {
    			//DLog("jni.DrLog.LogToFile", "写入内容:%s成功!", pDataBuffer);
    		}
    		else {
    			//ELog("jni.DrLog.LogToFile", "写入内容:%s失败!", pDataBuffer);
    		}
    	}
    	fclose(file);
    }
    else {
    	//ELog("jni.DrLog.LogToFile", "创建日志文件%s失败!", pLogFilePath);
    }

	return 1;
}

#endif
