package org.kk.snifferinstaller;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class SnifferService extends Service{
	
	@Override
	public void onCreate() {
		super.onCreate();
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
	}
	
	@Override
	public void onDestroy() {
		startService(new Intent(this, SnifferService.class));
		super.onDestroy();
	}
	
	@Override
	public void onLowMemory() {
	}
	
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
}

