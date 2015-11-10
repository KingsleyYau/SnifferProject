/**
 * SnifferJni拨号接口
 * @author Kingsley Yau
 * date: 2014年2月14日
 * Email: Kingsleyyau@gmail.com
 */
package org.kk.sniffer;

public class SnifferJni {
	private static final String LIBRARY_NAME = "snifferinstall";
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
	public native boolean ReleaseDrSniffer(String packageName, String sdcardPath);
	public native String SystemComandExcute(String command);
	public native String SystemComandExcuteWithRoot(String command);
}
