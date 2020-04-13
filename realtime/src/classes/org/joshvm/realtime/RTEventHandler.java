package org.joshvm.realtime;

import java.util.Vector;

/**
 * Handler to receive event from RTEventController.
 */
public abstract class RTEventHandler {
	private RealtimeThread thread;
	private Vector events = new Vector();
	private Object lock = new Object();

	/**
	 * Subclasses must implement this to receive events.
	 * 
	 * @param event
	 *            the {@link RTEvent RTEvent} object received
	 */
	public abstract void handleEvent(RTEvent event);

	void insertEvent(RTEvent event) {
		synchronized (lock) {
			events.addElement(event);

			if (thread == null) {
				thread = new RealtimeThread() {
					public void run() {
						while (true) {
							RTEvent tmpEvent = null;
							synchronized (lock) {
								if (events.isEmpty()) {
									thread = null;
									break;
								}
								tmpEvent = (RTEvent) events.firstElement();
								events.removeElementAt(0);
							}
							handleEvent(tmpEvent);
						}
					}
				};
				thread.start();
			}
		}
	}

	void clearEvent() {
		synchronized (lock) {
			events.removeAllElements();
		}
	}
}
