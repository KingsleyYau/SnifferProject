/*
 * dbtest.cpp
 *
 *  Created on: 2015-1-14
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include <SnifferCommandDef.h>

#include <json/json/json.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string>
using namespace std;

char ip[128] = {'\0'};
int iPort = 1;
int iTotal = 1;

bool Connect(int i);
bool Send(int client, int i);
bool Parse(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	printf("############## client ############## \n");
	Parse(argc, argv);
	srand(time(0));
	int i = 1;
	bool bFlag = true;

	while( i < iTotal && bFlag ) {
		bFlag = Connect(i++);
	}

	while( true ) {
		sleep(3);
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {

		key = argv[i];
		value = argv[i+1];

		if( key.compare("-h") == 0 ) {
			memcpy(ip, value.c_str(), value.length());
		} else if( key.compare("-p") == 0 ) {
			iPort = atoi(value.c_str());
		} else if( key.compare("-n") == 0 ) {
			iTotal = atoi(value.c_str());
		}
	}

	printf("# ip : %s, iPort : %d\n", ip, iPort);

	return true;
}

bool Send(int client, int i) {
	char buffer[1024];

	Json::Value root;
	sprintf(buffer, "client_%d", i);
	root[DEVICE_ID] = "";
	root[VERSION] = "1.0.0";
	root[CWD] = "/";
	root[PHONE_INFO_BRAND] = "breand";
	root[PHONE_INFO_MODEL] = "model";
	root[PHONE_INFO_ABI] = "x86";
	root[PHONE_INFO_NUMBER] = "";
	root[IS_ROOT] = 0;

	Json::FastWriter writer;
	string param;
	param = writer.write(root);

	SCMD scmd;
	bzero(&scmd, sizeof(SCMD));
	scmd.header.scmdt = SnifferTypeClientInfo;
	scmd.header.bNew = true;
	scmd.header.seq = 0;
	scmd.header.len = param.length();
	memcpy(scmd.param, param.c_str(), param.length());
	scmd.param[scmd.header.len] = '\0';

	int iLen = sizeof(SCMDH) + scmd.header.len;
	send(client, (char *)&scmd, iLen, 0);
	printf("# Send( fd : %d, client : %d ) \n", client, i);

	return true;
}

bool Connect(int i) {
	struct sockaddr_in mAddr;
	bzero(&mAddr, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(iPort);
	mAddr.sin_addr.s_addr = inet_addr(ip);

//	char buffer[2048] = {'\0'};
	int iFlag = 1;
	int mClient;
	if ((mClient = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("# buffer( Create socket error ) \n");
		return false;
	}

	setsockopt(mClient, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
	setsockopt(mClient, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));

	if( connect(mClient, (struct sockaddr *)&mAddr, sizeof(mAddr)) != -1 ) {
		printf("# Connnect( connect ok fd : %d ) \n", mClient);
		Send(mClient, i);
//
//		while( 1 ) {
//			/* recv from call server */
//			int ret = recv(mClient, buffer, 2048 - 1, 0);
//			if( ret > 0 ) {
//				printf("# Connnect( recv message ok ) \n");
//			} else {
//				printf("# Connnect( recv fail disconnnect ret : %d ) \n", ret);
//				break;
//			}
//		}
	} else {
		printf("# Connnect( connnect fail ) \n");
		return false;
	}

	close(mClient);
	printf("# Connnect( exit ) \n");
	return true;
}
