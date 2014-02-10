package net.bonrry.babyfoot.adk;


import net.bonrry.babyfoot.BabyfootService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;


public class UsbBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		Intent i = new Intent(context, BabyfootService.class);
	    context.startService(i);
	}
}
