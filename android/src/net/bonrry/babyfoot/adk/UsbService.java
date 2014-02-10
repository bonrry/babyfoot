package net.bonrry.babyfoot.adk;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

import android.app.Notification;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;

public abstract class UsbService extends Service {

	private final IBinder mBinder = new LocalBinder();
	private UsbManager usbManager;
	UsbAccessory accessory;
	ParcelFileDescriptor accessoryFileDescriptor;
	FileInputStream accessoryInput;
	FileOutputStream accessoryOutput;
	Thread mReadThread = null;
	static Handler messageHandler;

	ArrayList<AccessoryInterface> listeners = new ArrayList<AccessoryInterface>();

	public static final int TYPE_IN_MSG       = 1;
	public static final int TYPE_IO_ERROR_MSG = 2;

	private static final int ONGOING_NOTIFICATION = 424242;

	private static final String TAG = "UsbService";

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {

	    	Log.d(TAG, "onStartCommand");
	    	// Attach to accessory if one is connected
		attachAccessoryIfAny();

		// We want this service to continue running until it is explicitly
		// stopped, so return sticky.		
		return START_STICKY;
	}

	@Override
	public IBinder onBind(Intent intent) {
		Log.d(TAG, "onbind");
		return mBinder;
	}

	@Override
	public boolean onUnbind(Intent intent) {
		super.onUnbind(intent);
		Log.d(TAG, "onUnbind");
		return true;
	}

	@Override
	public void onRebind(Intent intent) {
		super.onRebind(intent);
		Log.d(TAG, "onRebind");
		attachAccessoryIfAny();
	}

	@Override
	public void onCreate() {
		super.onCreate();
		Log.d(TAG, "onCreate");

		messageHandler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				switch (msg.what) {
				case TYPE_IN_MSG:
					InMessage m = (InMessage) msg.obj;
					newMessageFromAccessory(m);
					break;
				case TYPE_IO_ERROR_MSG:
					Log.i(TAG, "Got message TYPE_IO_ERROR_MSG");
					closeAccessory();
					// TODO display error or finish()...
					break;
				}
			}
		};

		// Register to USB DETACH event (not ATTACHED as it is only sent to activity...)
		usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
		IntentFilter filter = new IntentFilter(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		registerReceiver(usbBroadcastReceiver, filter);

		//attachAccessoryIfAny();
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(usbBroadcastReceiver);
		closeAccessory();
		super.onDestroy();
	}

	/**
	 * Class used for the client Binder.  Because we know this service always
	 * runs in the same process as its clients, we don't need to deal with IPC.
	 */
	public class LocalBinder extends Binder {
		UsbService getService() {
			// Return this instance of LocalService so clients can call public methods
			return UsbService.this;
		}
	}


	/**
	 * ************************************************************************
	 * 						USB ACCESSORY STUFF
	 * ************************************************************************
	 */

	private void attachAccessoryIfAny() {
		Log.d(TAG, "attachAccessoryIfAny");
		UsbAccessory[] accessories = usbManager.getAccessoryList();
		accessory = (accessories == null ? null : accessories[0]);
		Log.d(TAG, "attachAccessoryIfAny: nb accessories=" + (accessories == null ? 0 : accessories.length));
		if (accessory != null) {
			openAccessory(accessory);
		}
	}

	private final BroadcastReceiver usbBroadcastReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
				Log.d(TAG, "usbBroadcastReceiver: received ACTION_USB_ACCESSORY_DETACHED");
				closeAccessory();
			}
		}
	};

	private void openAccessory(UsbAccessory accessory) {
		if (accessoryFileDescriptor != null) {
			Log.d(TAG, "openAccessory: already opened...");
			for (AccessoryInterface listener : listeners) {
				listener.accessoryConnected();
			}
			return;
		}
		Log.d(TAG, "openAccessory");
		accessoryFileDescriptor = usbManager.openAccessory(accessory);
		if (accessoryFileDescriptor != null) {
			this.accessory = accessory;
			FileDescriptor fd = accessoryFileDescriptor.getFileDescriptor();
			accessoryInput = new FileInputStream(fd);
			accessoryOutput = new FileOutputStream(fd);
			mReadThread = new Thread(null, new AccesoryReadThread(accessoryInput, messageHandler), "AdkReadThread");
			Log.d(TAG, "openAccessory: accessory opened");
			mReadThread.start();
			// TODO: enable USB operations in the app
			startForeground(ONGOING_NOTIFICATION, getNotification());
			for (AccessoryInterface listener : listeners) {
				listener.accessoryConnected();
			}
		} else {
			Log.d(TAG, "openAccessory: accessory open fail");
			this.accessory = null;
		}
	}

	private void closeAccessory() {
		Log.d(TAG, "closeAccessory");
		// TODO: disable USB operations in the app
		try {
			if (mReadThread != null && !mReadThread.isInterrupted())
				mReadThread.interrupt();
			if (accessoryFileDescriptor != null)
				accessoryFileDescriptor.close();
			Log.d(TAG, "closeAccessory: accessory closed");
			for (AccessoryInterface listener : listeners) {
				listener.accessoryDisconnected();
			}
		} catch (IOException e) {
			Log.e(TAG, "closeAccessory: "+e);
		}
		finally {
			accessoryFileDescriptor = null;
			accessory = null;
			stopForeground(true);
			stopSelf(); // Will actually stop AFTER all clients unbind... 
		}
	}

	protected abstract Notification getNotification();

	/**
	 * ************************************************************************
	 * 						Available to activities:
	 * ************************************************************************
	 */

	public void sendMessage(final byte[] buf, final int len) {
		Thread writeThread = new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					accessoryOutput.write(buf, 0, len);
				} catch (IOException e) {
					Log.d(TAG, "Exception in USB accessory input writing", e);
					closeAccessory();
				} catch (NullPointerException e) {
					Log.d(TAG, "USB accessory was closed, can't write in it.", e);
					closeAccessory();
				}
			}
		});
		writeThread.start();
	}

	public void listenAccessoryEvents(AccessoryInterface listener) {
		if (!listeners.contains(listener)) {
		    	listeners.add(listener);
		    	// If we are connected to an accessory, indicate it to the new listener
		    	if (this.accessory != null) {
		    	    	listener.accessoryConnected();
		    	}
		}
	}
	
	public void unlistenAccessoryEvents(AccessoryInterface listener) {
		if (listeners.contains(listener))
			listeners.remove(listener);
	}

	protected void newMessageFromAccessory(InMessage m) {
		if (m.len >= 0) {
			// Notify listeners that new data arrived
			for (AccessoryInterface listener : listeners) {
				listener.newAccessoryData(m);
			}			
		}
	}
}