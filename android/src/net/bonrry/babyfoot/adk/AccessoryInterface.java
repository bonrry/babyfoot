package net.bonrry.babyfoot.adk;

public interface AccessoryInterface {
	
	public void newAccessoryData(InMessage msg);
	
	public void accessoryConnected();

	public void accessoryDisconnected();
}
