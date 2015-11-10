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
#include "CommandDef.h"

class Client : public DataParser {
public:
	Client();
	virtual ~Client();

	int ParseData(char* buffer, int len);

	int fd;

    string brand;
    string model;
    string phoneNumber;

    SSCMD sscmd;
private:
	int type;
    Message message;

};

#endif /* CLIENT_H_ */
