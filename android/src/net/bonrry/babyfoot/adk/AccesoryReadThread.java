package net.bonrry.babyfoot.adk;

import java.io.FileInputStream;
import java.io.IOException;


import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class AccesoryReadThread implements Runnable {

	static final int BUF_LEN = 512;
	static final String TAG = "AccesoryReadThread";
	FileInputStream accessoryInput;
	Handler handler;
	
	public AccesoryReadThread(FileInputStream accessoryInput, Handler handler) {
		this.accessoryInput = accessoryInput;
		this.handler = handler;
	}
	
	/**
	 * Main USB reading loop, processing incoming data from accessory
	 */
	@Override
	public void run()
	{
		int ret = 0;
		byte[] buffer = new byte[BUF_LEN];
		
		Log.d(TAG, "ReadThread starting...");
		while (ret >= 0) {
			try {
				ret = accessoryInput.read(buffer);
				if (ret > 0) {
					//Log.d(TAG, "READ("+ret+")="+new String(buffer));
					Message m = Message.obtain(handler, UsbService.TYPE_IN_MSG);
					m.obj = new InMessage(buffer, ret);
					handler.sendMessage(m);
				}
			} catch (IOException e) {
				Log.d(TAG, "Exception in USB accessory input reading", e);
				Message m = Message.obtain(handler, UsbService.TYPE_IO_ERROR_MSG);
				handler.sendMessage(m);
				break;
			}
		}
		Log.e(TAG, "ReadThread aborting...");
	}
}
