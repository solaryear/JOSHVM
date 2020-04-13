package org.joshvm.realtime;

/**
 * A RTEvent can have a set of handlers associated with it, and when the event
 * occurs, the handler will be called.
 * 
 * @see RTEventController
 * @see RTEventHandler
 */
public class RTEvent {
	/**
	 * name of event
	 */
	public String name;

	/**
	 * Creates a new RTEvent
	 */
	public RTEvent() {
	}

	/**
	 * Creates a new RTEvent with the specified name.
	 * 
	 * @param name
	 *            name of event
	 */
	public RTEvent(String name) {
		this.name = name;
	}
}
