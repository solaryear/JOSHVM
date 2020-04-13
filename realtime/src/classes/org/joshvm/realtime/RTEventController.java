package org.joshvm.realtime;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

/**
 * RTEventController is used to manage RTEvent handlers.
 */
public class RTEventController {
	private static final RTEventController INSTANCE = new RTEventController();
	private Hashtable handlerTable;
	private RealtimeThread thread;
	private long lastTime = -1;
	private int lastIndex = -1;

	private RTEventController() {
	}

	/**
	 * Returns RTEventController instance
	 */
	public static RTEventController getInstance() {
		return INSTANCE;
	}

	private void checkName(String name) {
		if (name == null || name.length() == 0) {
			throw new IllegalArgumentException("Invalid name.");
		}
	}

	private void checkHandler(RTEventHandler handler) {
		if (handler == null) {
			throw new IllegalArgumentException("Invalid handler.");
		}
	}

	private boolean hasHandler() {
		return (handlerTable != null) && (handlerTable.size() > 0);
	}

	private void controlThread() {
		boolean shouldRunning = hasHandler();
		if (shouldRunning && thread == null) {
			thread = new RealtimeThread() {
				public void run() {
					while (true) {
						RTEvent event = new RTEvent();
						boolean result = getRTEvent(event);
						synchronized (INSTANCE) {
							if (hasHandler()) {
								if (result) {
									dispatchEvent(event);
								}
							} else {
								lastTime = -1;
								thread = null;
								break;
							}
						}
					}
				}
			};
			thread.start();
		} else if (!shouldRunning && thread != null) {
			interruptGetRTEvent0();
		}
	}

	/**
	 * Add a handler for the event specified by name.
	 * 
	 * @param name
	 *            name of event, must not be null.
	 * @param handler
	 *            handler of event, must not be null.
	 */
	public synchronized void addHandler(String name, RTEventHandler handler) {
		checkName(name);
		checkHandler(handler);

		if (handlerTable == null) {
			handlerTable = new Hashtable();
		}

		boolean toAdd = true;
		Vector vector = (Vector) handlerTable.get(name);
		if (vector == null) {
			vector = new Vector();
		} else {
			if (vector.contains(handler)) {
				// ignore
				toAdd = false;
			}
		}

		if (toAdd) {
			vector.addElement(handler);
			handlerTable.put(name, vector);
		}

		controlThread();
	}

	/**
	 * Remove all handlers associated with the event specified by name.
	 * 
	 * @param name
	 *            name of event, must not be null.
	 */
	public synchronized void removeHandler(String name) {
		checkName(name);

		if (handlerTable != null) {
			Vector vector = (Vector) handlerTable.get(name);
			if (vector == null) {
				return;
			}
			Enumeration e = vector.elements();
			while (e.hasMoreElements()) {
				RTEventHandler handler = (RTEventHandler) e.nextElement();
				handler.clearEvent();
			}
			handlerTable.remove(name);
		}
		controlThread();
	}

	/**
	 * Remove a handler from the set associated with the event specified by
	 * name.
	 * 
	 * @param name
	 *            name of event, must not be null.
	 * @param handler
	 *            handler of event, must not be null.
	 */
	public synchronized void removeHandler(String name, RTEventHandler handler) {
		checkName(name);
		checkHandler(handler);

		if (handlerTable != null) {
			Vector vector = (Vector) handlerTable.get(name);
			if (vector != null) {
				vector.removeElement(handler);
				handler.clearEvent();
				if (vector.isEmpty()) {
					handlerTable.remove(name);
				}
			}
		}
		controlThread();
	}

	/**
	 * Fire an event to make handlers associated with the event called.
	 * 
	 * @param event
	 *            the event to be fired
	 */
	public final void fireEvent(final RTEvent event) {
		new RealtimeThread() {
			public void run() {
				dispatchEvent(event);
			}
		}.start();
	}

	private synchronized void dispatchEvent(RTEvent event) {
		if (event == null) {
			return;
		}

		if (handlerTable == null) {
			return;
		}

		String name = event.name;
		Vector vector = (Vector) handlerTable.get(name);
		if (vector == null) {
			return;
		}
		Enumeration e = vector.elements();
		while (e.hasMoreElements()) {
			RTEventHandler handler = (RTEventHandler) e.nextElement();
			handler.insertEvent(event);
		}
	}

	private boolean getRTEvent(RTEvent event) {
		if (lastTime <= 0) {
			lastTime = System.currentTimeMillis();
		}
		int index = getRTEvent0(event, lastTime, lastIndex);
		if (index >= 0) {
			lastIndex = index;
			return true;
		}
		return false;
	}

	// for internal test
	private void fireEventIn(final String name) {
		new RealtimeThread() {
			public void run() {
				checkName(name);
				byte[] b = name.getBytes();
				fireEvent0(b, 0, b.length);
			}
		}.start();
	}

	private native int getRTEvent0(RTEvent event, long lastTime, int lastIndex);

	private native void interruptGetRTEvent0();

	private native void fireEvent0(byte[] name, int offset, int length);
}
