package org.kk.snifferinstaller;

import org.kk.sniffer.SnifferJni;

import android.app.Application;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;

public class SnifferApplication extends Application  {
	private SnifferJni jni = new SnifferJni();
	
	private static ServiceConnection serviceConnection = null;
	
	@Override
	public void onCreate() {
		super.onCreate();
        Log.d("sniffer", "SnifferApplication::onCreate");
        
		Intent intentPushService = new Intent(this, SnifferService.class);
		startService(intentPushService);
		
		serviceConnection = new ServiceConnection() {
			@Override
			public void onServiceConnected(ComponentName name, IBinder service) {
				// TODO Auto-generated method stub
				
			}
			@Override
			public void onServiceDisconnected(ComponentName name) {
				// TODO Auto-generated method stub
				
			}
		};
		
		Intent intentRemoteService = new Intent(this, SnifferRemoteService.class);
		startService(intentPushService);
		bindService(intentRemoteService, serviceConnection, Context.BIND_AUTO_CREATE);
		
        final String packageName = this.getPackageName(); 
        
        new Thread() {
        	@Override
			public void run() {
            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        		jni.ReleaseDrSniffer(packageName, sdcardPath);
        	}
        }.start();
        
	}
}
