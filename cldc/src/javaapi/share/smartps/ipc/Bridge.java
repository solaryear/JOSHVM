/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */

package smartps.ipc;

/**
 * This abstract class is the superclass of all classes representing
 * either an Inter-Thread or Inter-Application Bridge (i.e. non-named or named
 * Bridge).
 *
 * <p> Applications that need to use bridges need to create via open() method
 * with name or without name. If a named-bridge is created, another application
 * could use same name to open this bridge from its application, then use sink() 
 * or source() to get channel to send/receive data inter applications</p>
 *
 * @see     org.joshvm.util.ByteBuffer
 */
public abstract class Bridge {
	protected Bridge() {}

    /**
     * Open an Inter-Thread non-named Bridge.
     *
     * @return     the Bridge object for Inter-Thread communication
     */	
	public static synchronized Bridge open() {
		return new TransparentBridge();
	}

    /**
     * Open an Inter-Application named Bridge.
     *
     * @param      bridgeName   the name of the Bridge. Another application could use
     *                          same name to open this bridge from its own application size.
     * @return     the Bridge object for Inter-Application communication
     * @exception  IllegalArgumentException  if bridgeName is null
     */	
	public static synchronized Bridge open(String bridgeName) {
		if (bridgeName == null) {
			throw new IllegalArgumentException();
		}
		return new IpcBridge(bridgeName);
	}

	/**
	 * Open an Inter-Thread non-named Bridge with specifed buffer size.
	 *
	 * @param      bufferSize   specified size of the data buffer of the bridge 
	 * @return	   the Bridge object for Inter-Thread communication
	 * @exception  IllegalArgumentException  if bufferSize <= 0
	 */
	public static synchronized Bridge open(int bufferSize) {
		if (bufferSize <= 0) {
			throw new IllegalArgumentException();
		}
		return new TransparentBridge(bufferSize);
	}

	/**
	 * Open an Inter-Application named Bridge with specified buffer size.
	 *
	 * @param	   bridgeName	the name of the Bridge. Another application could use
	 *							same name to open this bridge from its own application size.
	 * @param      bufferSize   specified size of the data buffer of the bridge 
	 * @return	   the Bridge object for Inter-Application communication
	 * @exception  IllegalArgumentException  if bridgeName is null or bufferSize <= 0
	 */
	public static synchronized Bridge open(String bridgeName, int bufferSize) {
		if (bridgeName == null || bufferSize <= 0) {
			throw new IllegalArgumentException();
		}
		return new IpcBridge(bridgeName, bufferSize);
	}

	/**
	 * Get the data-exchange buffer size
	 *
	 * @return	   the buffer size of this Bridge object
	 */
	public abstract int getBufferSize();

	/**
	 * Get the sink channel of the bridge for sending data
	 *
	 * @return	   the buffer size of this Bridge object
	 */
	public abstract Bridge.SinkChannel sink();

	/**
	 * Get the source channel of the bridge for receiving data
	 *
	 * @return	   the buffer size of this Bridge object
	 */
	public abstract Bridge.SourceChannel source();

	public abstract class SinkChannel {
		/**
	 	* Write data from buffer to the channel.
	 	* If there's any pending data in the channel, write will return 0 and no data 
	 	* really written into the channel.
	 	*
	 	* @param       the buffer of data to be written into the channel
	 	* @return	   the actual written bytes of data. 0 if the channel is busy, need retry
	 	*/
		public abstract int write(org.joshvm.util.ByteBuffer buffer);
	}

	public abstract class SourceChannel {
		/**
	 	* Read data from the channel to the buffer
	 	*
	 	* @param       the buffer ready to receive the read data from the channel
	 	* @return	   the actual read bytes of data. 0 if no data available
	 	*/
		public abstract int read(org.joshvm.util.ByteBuffer buffer);
	}
}


