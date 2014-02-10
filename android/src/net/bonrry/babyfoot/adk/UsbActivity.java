package net.bonrry.babyfoot.adk;

import net.bonrry.babyfoot.adk.UsbService.LocalBinder;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;


public abstract class UsbActivity extends Activity implements AccessoryInterface {
	String TAG = "MainActivity";
	UsbService mService;
	boolean mBound = false;
	Intent usbServiceIntent;

	/**
	 * ************************************************************************
	 * 				Android APP life cycle
	 * ************************************************************************
	 */

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		usbServiceIntent = new Intent(this, getServiceClass());
	}
	
	protected abstract Class<?> getServiceClass();

	/*
	 * Defines callbacks for service binding, passed to bindService()
	 */
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName className,
				IBinder service) {
			// We've bound to LocalService, cast the IBinder and get LocalService instance
			LocalBinder binder = (LocalBinder) service;
			mService = binder.getService();
			mBound = true;
			mService.listenAccessoryEvents(UsbActivity.this);
		}

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			mBound = false;
		}
	};

	@Override 
	protected void onNewIntent(Intent intent) {
		Log.d(TAG, "onNewIntent: bind...");
		bindService(usbServiceIntent, mConnection, Context.BIND_ABOVE_CLIENT | BIND_AUTO_CREATE);
	};

	@Override
	protected void onStart() {
		super.onStart();
		bindService(usbServiceIntent, mConnection, Context.BIND_ABOVE_CLIENT | BIND_AUTO_CREATE);
	};

	@Override
	protected void onStop() {
		super.onStop();
		// Unbind from the service
		if (mBound) {
			mService.unlistenAccessoryEvents(UsbActivity.this);
			unbindService(mConnection);
			mBound = false;
		}
	};
	/**
	 * ************************************************************************
	 * 						Data Management
	 * ************************************************************************
	 */

	protected void sendAccessoryMessage(final byte[] buf) {
		if (mBound) {
		    	// Assume the first byte contains the size of the content of the message. Total size is contentSize + 1 (for the first byte)
			mService.sendMessage(buf, buf[0] + 1);
		}
	}

	protected void sendAccessoryMessage(final byte[] buf, final int len) {
		if (mBound) {
			mService.sendMessage(buf, len);
		}
	}
}
