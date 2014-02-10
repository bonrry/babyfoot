package net.bonrry.babyfoot;

public class BabyEvent {

		public BabyEventType type;
		public boolean isBlue; 
		public String time;
		public int scoreDiff;
		
		public BabyEvent(BabyEventType type, boolean isBlue, String time, int scoreDiff) {
			this.type = type;
			this.isBlue = isBlue;
			this.scoreDiff = scoreDiff;
			this.time = time;
		}
}
