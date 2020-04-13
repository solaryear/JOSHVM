package org.joshvm.realtime;

/**
 * A RealtimeThread is a {@link java.lang.Thread thread} with highest priority.
 * 
 * It will preempt other regular Java threads, so using it with high degree of
 * care.
 */
public class RealtimeThread extends Thread {
	/**
	 * Causes this thread to begin execution; the Java Virtual Machine calls the
	 * <code>run</code> method of this thread.
	 * 
	 * @exception IllegalThreadStateException
	 *                if the thread was already started.
	 * @see java.lang.Thread#run()
	 */
	public synchronized void start() {
		start(this);
	}

	private static synchronized void start(RealtimeThread thread) {
		thread.start0();
	}

	private native void start0();
}
