/**
 * DrSnifferJni拨号接口
 * @author Kingsley Yau
 * date: 2014年2月14日
 * Email: Kingsleyyau@gmail.com
 */
package com.drcom.drsniffer;

public class DrSnifferJni {
	private static final String LIBRARY_NAME = "drsnifferinstall";
	static{
		try{
			System.loadLibrary(LIBRARY_NAME);
		}catch( Exception e){
			e.printStackTrace();
		}

	}
	
	/*
	 * 安装客户端
	 */
	public native boolean IsRoot();
	public native boolean ReleaseDrSniffer(String sdcardPath);
	
	public native String SystemComandExcute(String command);
	public native String SystemComandExcuteWithRoot(String command);
	
	/*
	 * 服务端接口
	 */
	public native boolean StartServer();
	public native boolean StopServer();
	public native void GetOnlineClientList();
	public native void SendCommandToClient();
}
