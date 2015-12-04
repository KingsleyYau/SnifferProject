/*
 * SinfferExcuteDef.h
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SINFFEREXCUTEDEF_H_
#define SINFFEREXCUTEDEF_H_

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define RelaseFilePrefix				"/data/data/"
#define RelaseFileLib					"/lib/"

#define SinfferFile 					"sniffer"
#define SinfferVersionString 			"sniffer version "
#define SnifferVersion 					"1.0.0"
#define SnifferVersionEnd 				" "
#define AutoStartScriptFile				"install-recovery.sh"
#define SnifferInStallerFile			"libsniffer.so"

//#define ServerAdess					"sniffer.wicp.net"
#define ServerAdess						"192.168.30.150"
#define ServerPort						 9873
#define HttpServerPort					 9875
#define SnifferLogFileName				"sniffer"

static unsigned char AutoStartScript[] {
	0x23 ,0x21 ,0x2f ,0x73 ,0x79 ,0x73 ,0x74 ,0x65 ,0x6d ,0x2f ,0x62 ,0x69 ,0x6e ,0x2f ,0x73 ,0x68,
	0x0a ,0x64 ,0x72 ,0x73 ,0x6e ,0x69 ,0x66 ,0x66 ,0x65 ,0x72 ,0x20 ,0x26 ,0x0a,
};

#endif
