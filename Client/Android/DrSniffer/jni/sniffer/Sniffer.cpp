/*
 * Sniffer.cpp
 *
 *  Created on: 2014年3月3日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "Sniffer.h"

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <common/IPAddress.h>
#include <common/Arithmetic.h>
#include <common/KLog.h>

/*
 * 监听线程
 */
class SnifferRunnable : public KRunnable {
public:
	SnifferRunnable(Sniffer *pSniffer) {
		m_pSniffer = pSniffer;
	}
	virtual ~SnifferRunnable() {
		m_pSniffer = NULL;
	}
protected:
	void onRun() {
		while(1) {
			if(m_pSniffer && m_pSniffer->IsRunning()) {
				int iRet = m_pSniffer->SinfferData();
			}
			else {
				break;
			}
		}

	}
private:
	Sniffer *m_pSniffer;
};

Sniffer::Sniffer() {
	// TODO Auto-generated constructor stub
	mpSnifferThread = new KThread(new SnifferRunnable(this));
}

Sniffer::~Sniffer() {
	// TODO Auto-generated destructor stub
	if(mpSnifferThread) {
		delete mpSnifferThread;
		mpSnifferThread = NULL;
	}
}

bool Sniffer::StartSniffer(string deviceName) {
	if(IsRunning()) {
		FileLog(SnifferLogFileName, "Sniffer正在运行...");
		return true;
	}

	// 抓取数据链路层数据
	if((m_Socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
//	 抓取传输层数据
//	if((m_Socket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) {

		FileLog(SnifferLogFileName, "create socket error!");
	    return false;
	}

	// set to nonblocking
	int iFlag = 1;
	if ((iFlag = fcntl(m_Socket, F_GETFL, 0)) == -1) {
		FileLog(SnifferLogFileName, "fcntl F_GETFL socket error");
		return false;
	}
	if (fcntl(m_Socket, F_SETFL, iFlag | O_NONBLOCK) == -1) {
		FileLog(SnifferLogFileName, "fcntl F_SETFL socket error");
		return false;
	}

	struct ifreq ifr_ip;
	if(deviceName.length() == 0) {
		// 默认监听全部
		list<IpAddressNetworkInfo> deviceList = IPAddress::GetNetworkInfoList();
		for(list<IpAddressNetworkInfo>::iterator itr = deviceList.begin(); itr != deviceList.end(); itr++) {
			IpAddressNetworkInfo device = *itr;

			// 设置监听为混杂模式
			strcpy(ifr_ip.ifr_name, device.name.c_str());
			ifr_ip.ifr_flags = IFF_UP | IFF_PROMISC | IFF_BROADCAST | IFF_RUNNING;
			if (!(ioctl(m_Socket, SIOCSIFFLAGS, (char *) &ifr_ip))) {
				FileLog(SnifferLogFileName, "ioctl %s flag:0x%x success!", device.name.c_str(), ifr_ip.ifr_flags);
			}
			else {
				FileLog(SnifferLogFileName, "ioctl SIOCGIFFLAGS error!");
				return false;
			}
		}
	}
	else {
		// 设置监听为混杂模式
		strcpy(ifr_ip.ifr_name, deviceName.c_str());
		ifr_ip.ifr_flags = IFF_UP | IFF_PROMISC | IFF_BROADCAST | IFF_RUNNING;
		if (!(ioctl(m_Socket, SIOCSIFFLAGS, (char *) &ifr_ip))) {
			FileLog(SnifferLogFileName, "ioctl %s flag:0x%x success!", deviceName.c_str(), ifr_ip.ifr_flags);
		}
		else {
			FileLog(SnifferLogFileName, "ioctl SIOCGIFFLAGS error!");
			return false;
		}
	}

	m_bRunning = true;
	if(mpSnifferThread->start() > 0) {
		FileLog(SnifferLogFileName, "启动监听线程成功!");
	}

//	ILog("jni.Sniffer.StartSniffer", "开始监听网卡数据!");
	return true;
}
void Sniffer::StopSniffer() {
	m_bRunning = false;
	if(mpSnifferThread) {
		mpSnifferThread->stop();
	}
	FileLog(SnifferLogFileName, "停止监听网卡数据!");
}
bool Sniffer::IsRunning() {
	return m_bRunning;
}
void Sniffer::ResetParam() {

}
int Sniffer::SinfferData() {
	FileLog(SnifferLogFileName, "等待数据...");
	int iRet = -1;

	sockaddr_in remoteAd;
	bzero(&remoteAd, sizeof(remoteAd));
	int iRemoteAdLen = sizeof(sockaddr_in);

	unsigned char buff[2048];
	memset(buff, '\0', sizeof(buff));

	iRet = recvfrom(m_Socket, buff, sizeof(buff), 0, (struct sockaddr *)&remoteAd, &iRemoteAdLen);

	timeval tout;
	tout.tv_sec = 3;
	tout.tv_usec = 0;

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(m_Socket, &rset);
	int iRetS = select(m_Socket + 1, &rset, NULL, NULL, &tout);

	if(iRetS > 0) {
		iRet = recvfrom(m_Socket, buff, sizeof(buff), 0, (struct sockaddr *)&remoteAd, &iRemoteAdLen);

		if(iRet > 0) {
			AnalyseData(buff, iRet);
		}
	}
	else if(iRetS == 0){
		iRet = 0;
//		showLog("Jni.Sniffer.SinfferData", "Sinffer no data for 3 second!");
	}
	else {
//		ILog("Jni.Sniffer.SinfferData", "Sinffer data error!");
	}

	return iRet;
}
void Sniffer::AnalyseData(unsigned char* pBuffer, int iLen) {
	FileLog(SnifferLogFileName, "############################################################");
	struct ethhdr *ether;
	string result = "";
	// 打印数据链路层头部
	if(iLen > sizeof(struct ethhdr)) {
//		FileLog(SnifferLogFileName, "数据链路层头部---------------------------------------------");

		ether = (struct ethhdr *)pBuffer;

		switch(ether->h_proto) {
		case ETH_P_PPP_DISC:{
			// PPPoE discovery messages
			result = "(PPPoE discovery)";
		}break;
		case ETH_P_PPP_SES:{
			// PPPoE session messages
			result = "(PPPoE session)";
		}break;
		default: {
			break;
		}
		}

//		FileLog(SnifferLogFileName, "Protocol:0x%x%s", ether->h_proto, result.c_str());
//		result = Arithmetic::AsciiToHexWithSep((const char*)ether->h_source, sizeof(ether->h_source));
//		FileLog(SnifferLogFileName, "Source Max:%s ", result.c_str());
//		result = Arithmetic::AsciiToHexWithSep((const char*)ether->h_dest, sizeof(ether->h_dest));
//		FileLog(SnifferLogFileName, "Dest Max:%s ", result.c_str());

		AnalyseIPData(pBuffer + sizeof(struct ethhdr), iLen - sizeof(struct ethhdr));
	}
	FileLog(SnifferLogFileName, "############################################################");
}
void Sniffer::AnalyseIPData(unsigned char* pBuffer, int iLen) {
	struct iphdr *ip;
	string result = "";
	// 打印传输层头部
	if(iLen > sizeof(struct iphdr)) {
		FileLog(SnifferLogFileName, "传输层头部---------------------------------------------");

		ip = (struct iphdr *)pBuffer;
		switch(ip->protocol) {
		case IPPROTO_TCP :{
			// Tcp
			result = "(Tcp)";
			AnalyseTcpData(pBuffer + sizeof(struct iphdr),  iLen - sizeof(struct iphdr));
		}break;
		case IPPROTO_UDP:{
			// Udp
			result = "(Udp)";
			AnalyseUdpData(pBuffer + sizeof(struct iphdr),  iLen - sizeof(struct iphdr));
		}break;
		default:{
//			showLog("jni.Sniffer.AnalyseIPData", "其他类型的包不处理");
		}break;
		}

		FileLog(SnifferLogFileName, "Protocol : %d %s", ip->protocol ,result.c_str());
		FileLog(SnifferLogFileName, "Source IP : %s ", inet_ntoa(*(struct in_addr *)&ip->saddr));
		FileLog(SnifferLogFileName, "Dest IP : %s ", inet_ntoa(*(struct in_addr *)&ip->daddr));

	}
}
void Sniffer::AnalyseTcpData(unsigned char* pBuffer, int iLen) {
	// 打印Tcp头部
	struct tcphdr *tcp;
	if(iLen > sizeof(struct tcphdr)) {
		FileLog(SnifferLogFileName, "Tcp头部---------------------------------------------");
		tcp = (struct tcphdr *)pBuffer;
		FileLog(SnifferLogFileName, "Source Port:%d ", ntohs(tcp->source));
		FileLog(SnifferLogFileName, "Dest Port:%d ", ntohs(tcp->dest));
	}
}
void Sniffer::AnalyseUdpData(unsigned char* pBuffer, int iLen) {
	// 打印Udp头部
	struct udphdr *udp;
	if(iLen > sizeof(struct udphdr)) {
		FileLog(SnifferLogFileName, "打印Udp头部---------------------------------------------");
		udp = (struct udphdr *)pBuffer;
		FileLog(SnifferLogFileName, "Source Port: %d", ntohs(udp->source));
		FileLog(SnifferLogFileName, "Dest Port: %d", ntohs(udp->dest));
		FileLog(SnifferLogFileName, "Len : %d", ntohs(udp->len));
		FileLog(SnifferLogFileName, "Check : %d", ntohs(udp->check));
	}
}
