package org.kk.snifferinstaller;

import org.kk.sniffer.SnifferJni;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.util.Log;

/**
 * 接收系统广播广播
 *
 */
public class SnifferReceiver extends BroadcastReceiver {
	private SnifferJni jni = new SnifferJni();
	
	@Override
	public void onReceive(Context context, Intent intent) {
		Log.d("sniffer", "SnifferReceiver::onReceive( "
				+ "intent.getAction()"
				+ intent.getAction()
				+ " )"
				);
		
		if (intent.getAction().equals(Intent.ACTION_USER_PRESENT)) {
			// 解除锁屏
	        final String packageName = context.getPackageName(); 
	        
	        new Thread() {
	        	@Override
				public void run(){
	            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
	        		jni.ReleaseDrSniffer(packageName, sdcardPath);
	        	}
	        }.start();
		}
	}
}




