package com.drcom.drsnifferinstaller;

import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.view.Menu;

import com.drcom.drsniffer.DrSnifferJni;

public class MainActivity extends Activity {

	private DrSnifferJni jni = new DrSnifferJni();
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        new Thread() {
        	@Override
			public void run(){
            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        		jni.ReleaseDrSniffer(sdcardPath);
        	}
        }.start();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
