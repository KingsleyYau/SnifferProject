/*
 * Sniffer.h
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SNIFFER_H_
#define SNIFFER_H_

#include <common/KThread.h>
#include <common/KSocket.h>

#include <string>
using namespace std;

class Sniffer : public KSocket {
public:
	Sniffer();
	virtual ~Sniffer();

	// 函数
	bool StartSniffer(string deviceName = "");
	void StopSniffer();
	bool IsRunning();
	void ResetParam();

	int SinfferData();
	void AnalyseData(unsigned char* pBuffer, int iLen);
	void AnalyseIPData(unsigned char* pBuffer, int iLen);
	void AnalyseTcpData(unsigned char* pBuffer, int iLen);
	void AnalyseUdpData(unsigned char* pBuffer, int iLen);
private:
	KThread *mpSnifferThread;
	bool m_bRunning;
};

#endif /* SNIFFER_H_ */
