package net.bonrry.babyfoot;


import net.bonrry.babyfoot.adk.InMessage;
import net.bonrry.babyfoot.adk.UsbActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;


public class DisconnectedActivity extends UsbActivity {

    private static final String TAG = "DisconnectedActivity";
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);
	Log.d(TAG, "onCreate");
	setContentView(R.layout.disconnected_activity);
    }

    @Override
    protected Class<?> getServiceClass() {
	return BabyfootService.class;
    }

    @Override
    public void newAccessoryData(InMessage msg) {
    }

    @Override
    public void accessoryConnected() {
	Intent intent = new Intent(this, ScoreActivity.class);
	intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
	intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
	startActivity(intent);
    }

    @Override
    public void accessoryDisconnected() {
    }
}