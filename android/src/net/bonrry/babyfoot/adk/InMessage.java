package net.bonrry.babyfoot.adk;

public class InMessage {
    	public byte[] buf;
	public byte command = 0;
	public int len = -1;
	
	public InMessage(byte[] inBuffer, int buflen) {
		if (inBuffer[0] >= 0 && (inBuffer[0] == (buflen - 1)) && inBuffer[1] > 0) {
			len = inBuffer[0] - 1;
			command = inBuffer[1];
			buf = new byte[len];
			for (int i = 0; i < len; i++)
				buf[i] = inBuffer[i + 2];	
		}
	}
}
