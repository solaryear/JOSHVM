package com.sun.cldc.isolate;

public class InterIsolateEvent {
	private static final int BROADCAST_ID = -1;
	
	public static native int waitEvent(int waitonID);

	public static native void sendEvent(int sendtoID, int value);
}
