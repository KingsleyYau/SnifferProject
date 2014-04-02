package com.drcom.drsniffer;

import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

import com.drcom.drsniffer.DrSnifferJni;

public class MainActivity extends Activity {

	private DrSnifferJni jni = new DrSnifferJni();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final String packageName = this.getPackageName(); 
        new Thread() {
        	@Override
			public void run(){
            	String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        		jni.ReleaseDrSniffer(packageName, sdcardPath);
        	}
        }.start();
        
        Button button1 = (Button)this.findViewById(R.id.button1);
        Button button2 = (Button)this.findViewById(R.id.button2);
        Button button3 = (Button)this.findViewById(R.id.button3);
        Button button4 = (Button)this.findViewById(R.id.button4);
        
        button1.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				jni.StartServer();
			}
        });
        button2.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				jni.StopServer();
			}
        });
        button3.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				jni.SendCommandToClient();
			}
        });
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
