/*
 * Client.h
 *
 *  Created on: 2015-11-10
 *      Author: Max
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
using namespace std;

#include "MessageList.h"
#include "DataParser.h"
#include "SnifferCommandDef.h"
#include "LogManager.h"

#include "KSafeList.h"
#include "KSafeMap.h"
#include "Arithmetic.hpp"

typedef KSafeList<SCMD*> CMDLIST;
typedef KSafeMap<int, SCMD*> CMDMAP;

class Client : public DataParser {
public:
	Client();
	virtual ~Client();

	int ParseData(char* buffer, int len);

	int fd;

    string brand;
    string model;
    string phoneNumber;

    CMDLIST cmdListRecv;
    CMDMAP cmdMapSend;

private:
	int type;
    Message message;

};

#endif /* CLIENT_H_ */
