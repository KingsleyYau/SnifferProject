/*
 * DataHttpParser.cpp
 *
 *  Created on: 2015-9-28
 *      Author: Max
 */

#include "DataHttpParser.h"

DataHttpParser::DataHttpParser() {
	// TODO Auto-generated constructor stub
	Reset();
}

DataHttpParser::~DataHttpParser() {
	// TODO Auto-generated destructor stub
}

void DataHttpParser::Reset() {
	miContentLength = -1;
	mHttpType = UNKNOW;
	miSendMaxSeq = -1;
	mPath = "";
	mParameters.clear();
}

int DataHttpParser::ParseData(char* buffer, int len) {
	int result = 0;
	int j = 0;

	// parse header
	char *pFirst = NULL;
	char*p = strtok_r(buffer, "\r\n", &pFirst);
	while( p != NULL ) {
		if( j == 0 ) {
			if( ParseFirstLine(p) ) {
				result = 1;
			}

			// only first line is useful
			break;
		}
		j++;
		p = strtok_r(NULL, "\r\n", &pFirst);
	}

//	if( !mbReceiveHeaderFinish ) {
//		int recvLen = (len < MAX_BUFFER_LEN - mHeaderIndex)?len:MAX_BUFFER_LEN - mHeaderIndex;
//		if( recvLen > 0 ) {
//			memcpy(mHeaderBuffer + mHeaderIndex, buffer, recvLen);
//			mHeaderIndex += recvLen;
//			mHeaderBuffer[mHeaderIndex + 1] = '\0';
//
//			// find Header Sep
//			char *pBody = strstr(mHeaderBuffer, "\r\n\r\n");
//			if( pBody != NULL ) {
//				pBody += strlen("\r\n\r\n");
//				mbReceiveHeaderFinish = true;
//
//				// parse header
//				char *pFirst = NULL;
//				char*p = strtok_r(mHeaderBuffer, "\r\n", &pFirst);
//				while( p != NULL ) {
//					if( j == 0 ) {
//						ParseFirstLine(p);
//						result = 1;
//						// only first line is useful
//						break;
//					} else {
//						if( (pParam = strstr(p, "Content-Length:")) != NULL ) {
//							int len = strlen("Content-Length:");
//							// find Content-Length
//							pParam += len;
//							if( pParam != NULL  ) {
//								memcpy(temp, pParam, strlen(p) - len);
//								miContentLength = atoi(temp);
//							}
//						}
//					}
//					j++;
//					p = strtok_r(NULL, "\r\n", &pFirst);
//				}
//			}
//		}
//	}


	return result;
}

const string& DataHttpParser::GetParam(const char* key) {
	string result = "";
	Parameters::iterator itr = mParameters.find(key);
	if( itr != mParameters.end() ) {
		result = (itr->second);
	}
	return result;
}

const string& DataHttpParser::GetPath() {
	return mPath;
}

HttpType DataHttpParser::GetType() {
	return mHttpType;
}

bool DataHttpParser::ParseFirstLine(char* buffer) {
	bool bFlag = true;
	char temp[1024];
	char* p = NULL;
	int j = 0;

	char *pFirst = NULL;

	p = strtok_r(buffer, " ", &pFirst);
	while( p != NULL ) {
		switch(j) {
		case 0:{
			// type
			if( strcmp("GET", p) == 0 ) {
				mHttpType = GET;
			} else if( strcmp("POST", p) == 0 ) {
				mHttpType = POST;
			} else {
				bFlag = false;
				break;
			}
		}break;
		case 1:{
			// path and parameters
			char path[1025] = {'\0'};
			Arithmetic ari;
			int len = strlen(p);
			len = ari.decode_url(p, len, temp);
			char* pPath = strstr(temp, "?");
			if( pPath != NULL && ((pPath + 1) != NULL) ) {
				len = pPath - temp;
				memcpy(path, temp, len);
				ParseParameters(pPath + 1);
			} else {
				memcpy(path, temp, len);
			}
			path[len] = '\0';
			mPath = path + 1;
//			transform(mPath.begin(), mPath.end(), mPath.begin(), ::toupper);
		}break;
		default:break;
		};

		j++;
		p = strtok_r(NULL, " ", &pFirst);
	}

	return bFlag;
}

void DataHttpParser::ParseParameters(char* buffer) {
	char* p = NULL;
	char* param = NULL;
	string key;
	string value;
//	int j = 0;

	char *pFirst = NULL;
//	char *pSecond = NULL;

	param = strtok_r(buffer, "&", &pFirst);
	while( param != NULL ) {
		p = strstr(param, "=");
		if( p != NULL && ((p + 1) != NULL) ) {
			*p = '\0';
			// key
			key = param;
			transform(key.begin(), key.end(), key.begin(), ::toupper);

			// value
			value = p + 1;
			mParameters.insert(Parameters::value_type(key, value));
		} else {
			// key
			key = p;
			transform(key.begin(), key.end(), key.begin(), ::toupper);
		}

//		j = 0;
//		p = strtok_r(param, "=", &pSecond);
//		while( p != NULL ) {
//			switch(j) {
//			case 0:{
//				// key
//				key = p;
//				transform(key.begin(), key.end(), key.begin(), ::toupper);
//			}break;
//			case 1:{
//				// value
//				value = p;
////				transform(value.begin(), value.end(), value.begin(), ::toupper);
//				mParameters.insert(Parameters::value_type(key, value));
//			}break;
//			default:break;
//			};
//
//			j++;
//			p = strtok_r(NULL, "=", &pSecond);
//		}
		param = strtok_r(NULL, "&", &pFirst);
	}
}

void DataHttpParser::SetSendMaxSeq(int seq) {
	mSeqMutex.lock();
	miSendMaxSeq = seq;
	mSeqMutex.unlock();
}

bool DataHttpParser::IsFinishSeq(int seq) {
	bool bFlag = false;

	mSeqMutex.lock();
	if( miSendMaxSeq == seq ) {
		bFlag = true;
	}
	mSeqMutex.unlock();

	return bFlag;
}
