package net.bonrry.babyfoot;


import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import net.bonrry.babyfoot.adk.InMessage;
import net.bonrry.babyfoot.adk.UsbActivity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.Chronometer;
import android.widget.ImageView;
import android.widget.TextView;


public class ScoreActivity extends UsbActivity {

	private static final String TAG = "ScoreActivity";

	//------------------------------------------------------------------------------
	// Commands for Arduino-Android communication
	//
	//                    !!! KEEP IN SYNC WITH ARDUINO CODE !!!
	//------------------------------------------------------------------------------
	// Commands format: <LENGTH><COMMAND><data...>
	// <LENGTH>  : 1 byte representing the LENGTH of the data (<LENGTH> byte not included)
	// <COMMAND> : 1 byte representing the command name (alls commands are described below)
	// <data>    : <LENGTH> - 1 bytes representing the data to transmit

	// GOAL: there is a goal. Len=2, format: 2g<TEAM>. <TEAM> is 'r' if the red scored, 'b' if the blue scored.
	// Example: "2gr" -> Red goal. "2gb" -> Blue goal
	private static final byte CMD_GOAL       = 'g';

	//------------------------------------------------------------------------------

	private static final int CLEAR        		= 'c';
	private static final int DONT_CHANGE  		= Integer.MAX_VALUE;
	private static final boolean GAME_STARTED  	= true;
	private static final boolean GAME_FINISHED  = false;

	private TextView 							txtScoreBlue, txtScoreRed, txtEndGame;
	private HorizontalListView 					eventListView;
	private Button 								btnUndo, btnLob, btnDemi, btnGamelleBlue, btnGamelleRed;
	private Chronometer 						chronometer;

	private ArrayList<BabyEvent> 				babyEvents;
	private BabyEventAdapter 					babyEventAdapter;

	private int scoreBlue;
	private int scoreRed;
	private int scoreFactor;

	private Runnable runnableEndMessage;
	private Handler handlerEndMessage = new Handler();

	private boolean gameStatus  = GAME_FINISHED;

	Animation animGoal;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.score_activity);
		txtScoreRed = (TextView) findViewById(R.id.txtRedScore);
		txtScoreBlue = (TextView) findViewById(R.id.txtBlueScore);
		txtEndGame = (TextView) findViewById(R.id.txtEndGame);
		eventListView = (HorizontalListView) findViewById(R.id.historic);
		btnUndo = (Button) findViewById(R.id.butUndoLastGoal);
		btnLob = (Button) findViewById(R.id.butSpecialGoalLob);
		btnDemi = (Button) findViewById(R.id.butSpecialGoalDemi);
		btnGamelleBlue = (Button) findViewById(R.id.butSpecialGoalGamelleBlue);
		btnGamelleRed = (Button) findViewById(R.id.butSpecialGoalGamelleRed);
		chronometer = (Chronometer) findViewById(R.id.chronometer);

		// event historic
		babyEvents = new ArrayList<BabyEvent>();
		babyEventAdapter = new BabyEventAdapter(babyEvents, getLayoutInflater());
		eventListView.setAdapter(babyEventAdapter);

		// action delay at the end of the game
		runnableEndMessage = new Runnable() {
			@Override
			public void run() {
				txtEndGame.setVisibility(View.GONE);
				if (scoreBlue == 10 || scoreRed == 10 && gameStatus == GAME_STARTED) {
					gameStatus = GAME_FINISHED;
					if (scoreBlue <= 0 || scoreRed <= 0) addBabyEvents(BabyEventType.FANNY, false, 0);
					addBabyEvents(BabyEventType.FINISH, false, 0);
					setStatusOfButtons();
					chronometer.stop();
				}
			}
		};

		// Animation du zoom pour les buts
		animGoal = AnimationUtils.loadAnimation(this, R.anim.zoom);
	}

	@Override
	protected Class<?> getServiceClass() {
		return BabyfootService.class;
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		super.onSaveInstanceState(outState);
	}

	@Override
	protected void onStart() {
		super.onStart();
		scoreBlue = scoreRed = 0;
		setScore(scoreBlue, scoreRed);
	}



	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// do something if needed :)
		return true;
	}


	private void setScore(int newScoreBlue, int newScoreRed) {
		if (newScoreBlue != DONT_CHANGE) {
			scoreBlue = newScoreBlue;
			txtScoreBlue.setTextColor(getResources().getColor(R.color.blueLight));
			txtScoreBlue.setText("" + scoreBlue);
			txtScoreBlue.startAnimation(animGoal);
		}
		if (newScoreRed != DONT_CHANGE) {
			scoreRed = newScoreRed;
			txtScoreRed.setTextColor(getResources().getColor(R.color.redLight));
			txtScoreRed.setText("" + scoreRed);
		}
		// si nombre de but au dessus de 10
		if (scoreBlue > 10 || scoreRed > 10) {
			if (babyEvents.size() <= 0) return;
			BabyEvent lastItem = babyEvents.get(babyEvents.size()-1);
			if (lastItem.type != BabyEventType.MORE10) {
				int diff = (scoreBlue > 10)?(scoreBlue - 10):(scoreRed - 10);
				setScore(scoreBlue - diff, scoreRed - diff);
				addBabyEvents(BabyEventType.MORE10, true, -diff);
				addBabyEvents(BabyEventType.MORE10, false, -diff);
			}
		}
		handlerEndMessage.removeCallbacks(runnableEndMessage);
		txtEndGame.setVisibility(View.GONE);
		if ((scoreBlue == 10 || scoreRed == 10) && gameStatus == GAME_STARTED) {
			txtEndGame.setVisibility(View.VISIBLE);
			handlerEndMessage.postDelayed(runnableEndMessage, 10000);
		}

		handler.sendEmptyMessageDelayed(CLEAR, 1000);
	}

	private void addBabyEvents(BabyEventType type, boolean isBlue, int scoreDiff) {
		BabyEvent event = new BabyEvent(type, isBlue, String.valueOf( convertSecondsToTimer((int)(SystemClock.elapsedRealtime() - chronometer.getBase()) / 1000 )) + "'", scoreDiff);
		babyEvents.add(event);
		babyEventAdapter.notifyDataSetChanged();
		eventListView.post(new Runnable(){
			public void run() {
				eventListView.setSelection(eventListView.getCount() - 1);
			}});
	}

	
	public void startNewGame(View v) {
		setScore(0,0);
		scoreFactor = 1;
		gameStatus = GAME_STARTED;
		babyEvents.clear();
		setStatusOfButtons();
		chronometer.setBase(SystemClock.elapsedRealtime());
		chronometer.start();
		addBabyEvents(BabyEventType.START, true, 0);
	}


	public void undoLastGoal(View v) {
		BabyEvent lastItem = babyEvents.get(babyEvents.size()-1);
		if (lastItem.type == BabyEventType.START) return;
		if (lastItem.isBlue) setScore(scoreBlue - Integer.valueOf(lastItem.scoreDiff), DONT_CHANGE);
		else setScore(DONT_CHANGE, scoreRed - Integer.valueOf(lastItem.scoreDiff));
		babyEvents.remove(babyEvents.size()-1);
		babyEventAdapter.notifyDataSetChanged();
		eventListView.post(new Runnable(){
			public void run() {
				eventListView.setSelection(eventListView.getCount() - 1);
			}});

		scoreFactor = 1;
		lastItem = babyEvents.get(babyEvents.size()-1);
		if (lastItem.type == BabyEventType.START) return;
		if (lastItem.type == BabyEventType.DEMI) scoreFactor = 2;
		else if (lastItem.type == BabyEventType.TRIMI) scoreFactor = 3;
		else if (lastItem.type == BabyEventType.FAULT) scoreFactor = 3;
		else if (lastItem.type == BabyEventType.GAMELLE) scoreFactor = Math.abs(Integer.valueOf(lastItem.scoreDiff));
	}

	public void specialGoalLob(View v) {
		BabyEvent lastItem;
		while (true) {
			lastItem = babyEvents.get(babyEvents.size()-1);
			if (lastItem.type == BabyEventType.MORE10) undoLastGoal(v);
			else break;
		}
		if (lastItem.type != BabyEventType.GOAL) return;
		if (lastItem.isBlue) setScore(scoreBlue + Integer.valueOf(lastItem.scoreDiff), DONT_CHANGE);
		else setScore(DONT_CHANGE, scoreRed + Integer.valueOf(lastItem.scoreDiff));
		addBabyEvents(BabyEventType.LOB, lastItem.isBlue, lastItem.scoreDiff);
	}

	public void specialGoalDemi(View v) {
		BabyEvent lastItem;
		while (true) {
			lastItem = babyEvents.get(babyEvents.size()-1);
			if (lastItem.type == BabyEventType.MORE10) undoLastGoal(v);
			else break;
		}

		if (lastItem.type != BabyEventType.GOAL) return;
		boolean wasBlue = lastItem.isBlue;
		undoLastGoal(v);

		scoreFactor++;
		if (scoreFactor == 2) addBabyEvents(BabyEventType.DEMI, wasBlue, 0);
		else if (scoreFactor == 3) addBabyEvents(BabyEventType.TRIMI, wasBlue, 0);
		else if (scoreFactor > 3) {
			scoreFactor = 3;
			if (wasBlue) setScore(scoreBlue - 2, DONT_CHANGE);
			else setScore(DONT_CHANGE, scoreRed - 2);
			addBabyEvents(BabyEventType.FAULT, wasBlue, -2);
		}
	}

	public void specialGoalGamelle(View v) {
		if (v.getId() == R.id.butSpecialGoalGamelleRed) {
			addBabyEvents(BabyEventType.GAMELLE, false, -scoreFactor);
			setScore(DONT_CHANGE, scoreRed - scoreFactor);
		}
		else if (v.getId() == R.id.butSpecialGoalGamelleBlue) {
			addBabyEvents(BabyEventType.GAMELLE, true, -scoreFactor);
			setScore(scoreBlue - scoreFactor, DONT_CHANGE);
		}
	}




	Handler handler = new Handler() {
		@Override
		public void handleMessage(Message message) {

			if (message.what == CLEAR) {
				txtScoreRed.setTextColor(getResources().getColor(R.color.redDark));
				txtScoreBlue.setTextColor(getResources().getColor(R.color.blueDark));		
				return;
			}

			InMessage msg = (InMessage) message.obj;
			switch (msg.command) {
			case CMD_GOAL :
				if (gameStatus == GAME_FINISHED)
					break;
				if (msg.buf[msg.len - 1] == 'b') {
					Log.d(TAG, "blue goal");
					addBabyEvents(BabyEventType.GOAL, true, scoreFactor);
					setScore(scoreBlue + scoreFactor, DONT_CHANGE);
					scoreFactor = 1;

				} else {
					Log.d(TAG, "red goal");
					addBabyEvents(BabyEventType.GOAL, false, scoreFactor);
					setScore(DONT_CHANGE, scoreRed + scoreFactor);
					scoreFactor = 1;
				}
				break;
			default:
				Log.e(TAG, "wrong message! " + msg.command);
			};
		}
	};

	private void setStatusOfButtons() {
		int visibility = (gameStatus == GAME_STARTED)?(View.VISIBLE):(View.GONE);
		boolean enabled = (gameStatus == GAME_STARTED)?(true):(false);
		btnUndo.setVisibility(visibility); btnUndo.setEnabled(enabled);
		btnDemi.setVisibility(visibility); btnDemi.setEnabled(enabled);
		btnLob.setVisibility(visibility); btnLob.setEnabled(enabled);
		btnGamelleBlue.setVisibility(visibility); btnGamelleBlue.setEnabled(enabled);
		btnGamelleRed.setVisibility(visibility); btnGamelleRed.setEnabled(enabled);
	}

	public static String convertSecondsToTimer(long seconds) {
		long minute = TimeUnit.SECONDS.toMinutes(seconds) - (TimeUnit.SECONDS.toHours(seconds)* 60);
		long second = TimeUnit.SECONDS.toSeconds(seconds) - (TimeUnit.SECONDS.toMinutes(seconds) *60);
		return String.format("%02d:%02d", minute, second);
	}

	@Override
	public void newAccessoryData(InMessage msg) {
		Message message = handler.obtainMessage(0, msg);
		handler.sendMessage(message);
	}

	@Override
	public void accessoryConnected() {
		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Log.d(TAG, "accessoryConnected");
			}
		});
	}

	@Override
	public void accessoryDisconnected() {
		Intent intent = new Intent(this, DisconnectedActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		startActivity(intent);
	}


	// SPECIFIC ADAPTER FOR THE EVENTS LINE
	private class BabyEventAdapter extends BaseAdapter {

		ArrayList<BabyEvent> babyEvents;
		LayoutInflater inflater;

		public BabyEventAdapter(ArrayList<BabyEvent> babyEvents, LayoutInflater inflater) {
			this.babyEvents = babyEvents;
			this.inflater = inflater;
		}

		@Override
		public int getCount() {
			return babyEvents.size();
		}

		@Override
		public Object getItem(int index) {
			return babyEvents.get(index);
		}

		@Override
		public long getItemId(int index) {
			return index;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			BabyEvent currentEvent = babyEvents.get(position);
			if (convertView == null) {
				convertView = inflater.inflate(R.layout.historic_item, parent, false);
			}
			((TextView) convertView.findViewById(R.id.timer)).setText(currentEvent.time);
			switch (currentEvent.type) {
			case GOAL:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_goal);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("GOAL (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_goal);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("GOAL (" + currentEvent.scoreDiff + ")");
				}
				break;
			case DEMI:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("DEMI (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("DEMI (" + currentEvent.scoreDiff + ")");
				}
				break;
			case TRIMI:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("TRIMI (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("TRIMI (" + currentEvent.scoreDiff + ")");
				}
				break;
			case GAMELLE:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("GAMELLE (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("GAMELLE (" + currentEvent.scoreDiff + ")");
				}
				break;
			case LOB:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("LOB (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("LOB (" + currentEvent.scoreDiff + ")");
				}
				break;
			case FAULT:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_redcard);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("FAULT (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_redcard);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("FAULT (" + currentEvent.scoreDiff + ")");
				}
				break;
			case FANNY:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_fanny);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("FANNY (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_fanny);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("FANNY (" + currentEvent.scoreDiff + ")");
				}
				break;
			case MORE10:
				if (currentEvent.isBlue) {
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreBlue)).setText("10+ (" + currentEvent.scoreDiff + ")");
				}
				else {
					((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_special);
					((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_empty);
					((TextView) convertView.findViewById(R.id.titreRed)).setText("10+ (" + currentEvent.scoreDiff + ")");
				}
				break;
			case START:
				((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_start);
				((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
				((TextView) convertView.findViewById(R.id.titreBlue)).setText("START" );
				break;
			case FINISH:
				((ImageView) convertView.findViewById(R.id.imgBlue)).setImageResource(R.drawable.ic_event_finish);
				((ImageView) convertView.findViewById(R.id.imgRed)).setImageResource(R.drawable.ic_event_empty);
				((TextView) convertView.findViewById(R.id.titreBlue)).setText("START" );
				break;
			default:
				break;
			}
			return convertView;
		}

	}



}