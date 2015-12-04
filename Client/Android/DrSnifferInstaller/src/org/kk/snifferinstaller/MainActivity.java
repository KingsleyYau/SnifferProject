package org.kk.snifferinstaller;

import org.kk.sniffer.SnifferJni;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;


public class MainActivity extends Activity {

	private SnifferJni jni = new SnifferJni();
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        final String packageName = this.getPackageName(); 

//        new Thread() {
//        	@Override
//			public void run(){
//            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
//        		jni.ReleaseDrSniffer(packageName, sdcardPath);
//        	}
//        }.start();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
