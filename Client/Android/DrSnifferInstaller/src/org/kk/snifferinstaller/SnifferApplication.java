package org.kk.snifferinstaller;

import org.kk.sniffer.SnifferJni;

import android.app.Application;
import android.os.Environment;

public class SnifferApplication extends Application  {
	private SnifferJni jni = new SnifferJni();
	
	@Override
	public void onCreate() {
		super.onCreate();
		
        final String packageName = this.getPackageName(); 

        new Thread() {
        	@Override
			public void run(){
            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        		jni.ReleaseDrSniffer(packageName, sdcardPath);
        	}
        }.start();
        
	}
}
