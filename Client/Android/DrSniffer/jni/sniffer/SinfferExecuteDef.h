/*
 * SinfferExcuteDef.h
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */
#ifndef SINFFEREXCUTEDEF_H_
#define SINFFEREXCUTEDEF_H_

#define RelaseFilePrefix				"/data/data/"
#define RelaseFileLib					"/lib/"

#define SinfferFile 					"sniffer"
#define SinfferVersionString 			"sniffer version "
#define SnifferVersion 					"1.1"
#define AutoStartScriptFile				"install-recovery.sh"
#define SnifferInStallerFile			"libsniffer.so"

#include <common/command.h>

unsigned char AutoStartScript[] {
	0x23 ,0x21 ,0x2f ,0x73 ,0x79 ,0x73 ,0x74 ,0x65 ,0x6d ,0x2f ,0x62 ,0x69 ,0x6e ,0x2f ,0x73 ,0x68,
	0x0a ,0x64 ,0x72 ,0x73 ,0x6e ,0x69 ,0x66 ,0x66 ,0x65 ,0x72 ,0x20 ,0x26 ,0x0a,
};

#endif
