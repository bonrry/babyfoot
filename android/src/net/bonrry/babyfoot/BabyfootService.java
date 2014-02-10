package net.bonrry.babyfoot;

import net.bonrry.babyfoot.adk.InMessage;
import net.bonrry.babyfoot.adk.UsbService;
import android.app.Notification;
import android.app.PendingIntent;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;


public class BabyfootService extends UsbService {

	private static final String TAG = "BabyfootService";

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// Show UI
		Intent uiActivityIntent = new Intent(this, ScoreActivity.class);
		uiActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		startActivity(uiActivityIntent);

		// FIXME : todo
		//handleCommand(intent);
		Log.d(TAG, "onStartCommand");
		Toast.makeText(this, getString(R.string.accessory_connected), Toast.LENGTH_SHORT).show();

		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy");
		super.onDestroy();
	}

	protected Notification getNotification() {
		Intent notificationIntent = new Intent(this, ScoreActivity.class);
		PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);
		
		Notification notification = new Notification.Builder(this)
        .setSmallIcon(R.drawable.ic_launcher)
        .setTicker(getText(R.string.notif_service_connected))
        .setWhen(System.currentTimeMillis())
        .setContentTitle(getText(R.string.notification_title))
        .setContentText(getText(R.string.notification_message))
        .setContentIntent(pendingIntent)
        .build();

		return notification;
	}

	/**
	 * ************************************************************************
	 * 						Available to activities:
	 * ************************************************************************
	 */

	protected void newMessageFromAccessory(InMessage m) {
	    
		if (m.len >= 0) {
			Log.v(TAG, "MSG received: (len: " + m.len + "), Command: " + m.command + " - Data: " + ((m.len > 0) ? new String(m.buf, 0, m.len) : "null"));
			if (m.len > 0 && m.command == 'M') {
				// TODO: parse message, store values in database... Do what a service should do ;)
			}		
		}
		super.newMessageFromAccessory(m);
	}
}