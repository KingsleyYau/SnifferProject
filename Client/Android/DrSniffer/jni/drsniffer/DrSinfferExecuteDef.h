/*
 * DrSinfferExcuteDef.h
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */
#ifndef DRSINFFEREXCUTEDEF_H_
#define DRSINFFEREXCUTEDEF_H_

#define DrRelaseFilePrefix				"/data/data/"
#define DrRelaseFileLib					"/lib/"

#define DrCurlFile 						"curl"
#define DrCurlFileVersionString 		"curl "
#define DrCurlVersion 					"7.34.0"
#define DrCurlInStallerFile				"libcurl.so"

#define DrSinfferFile 			"drsniffer"
#define DrSinfferVersionString 	"drsniffer version "
#define DrSnifferVersion 		"1.8"
#define AutoStartScriptFile		"install-recovery.sh"
#define DrSnifferInStallerFile	"libdrsniffer.so"

#include "../common/command.h"

/*
 * 1.2版本开始废弃
 */
const char DrSinfferExcute[] {
};

/*
 *
 */

unsigned char AutoStartScript[] {
	0x23 ,0x21 ,0x2f ,0x73 ,0x79 ,0x73 ,0x74 ,0x65 ,0x6d ,0x2f ,0x62 ,0x69 ,0x6e ,0x2f ,0x73 ,0x68,
	0x0a ,0x64 ,0x72 ,0x73 ,0x6e ,0x69 ,0x66 ,0x66 ,0x65 ,0x72 ,0x20 ,0x26 ,0x0a,
};
#endif
