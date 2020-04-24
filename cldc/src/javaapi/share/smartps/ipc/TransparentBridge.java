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

class TransparentBridge extends Bridge {
	private static int BUFFER_SIZE = 1024;
	private int bufferSize;
	
	private int bufferMark;
	private int readMark;
	
	private byte buffer[];

	private Bridge.SinkChannel sinkChannel;
	private Bridge.SourceChannel sourceChannel;
	
	TransparentBridge() {
		this(BUFFER_SIZE);
	}

	TransparentBridge(int bufferSize) {
		if (bufferSize <= 0) {
			throw new IllegalArgumentException();
		} else {
			this.bufferSize = bufferSize;
		}
		
		sinkChannel = null;
		sourceChannel = null;
		bufferMark = 0;
		readMark = 0;
		buffer = new byte[bufferSize];
	}

	public synchronized Bridge.SinkChannel sink() {
		if (sinkChannel == null) {
			sinkChannel = new TransparentBridge.SinkChannel(this);
		}
		return sinkChannel;
	}

	public synchronized Bridge.SourceChannel source() {
		if (sourceChannel == null) {
			sourceChannel = new TransparentBridge.SourceChannel(this);
		}
		return sourceChannel;
	}

	protected synchronized int write(org.joshvm.util.ByteBuffer writeBuffer) {
		if (bufferMark > 0) {
			return 0; //busy
		}
		
		if (writeBuffer.remaining() <= bufferSize) {
			bufferMark = writeBuffer.remaining();
		} else {
			bufferMark = bufferSize;
		}

		writeBuffer.get(buffer, 0, bufferMark);

		return bufferMark;
	}

	protected synchronized int read(org.joshvm.util.ByteBuffer readBuffer) {
		int readLen = bufferMark - readMark;

		if (readLen == 0) {
			return 0;
		}
		
		if (readBuffer.remaining() < readLen) {
			readLen = readBuffer.remaining();
		}
		readBuffer.put(new org.joshvm.util.ByteBuffer(buffer, readMark, readMark, readMark+readLen));

		readMark += readLen;

		if (readMark >= bufferMark) {
			//All read, reset the marks
			readMark = 0;
			bufferMark = 0;
		}
		
		return readLen;
	}

	public int getBufferSize() {
		return bufferSize;
	}

	class SinkChannel extends Bridge.SinkChannel {
		private TransparentBridge theBridge;
		
		SinkChannel(TransparentBridge bridge) {
			theBridge = bridge;
		}
		
		public int write(org.joshvm.util.ByteBuffer buffer) {
			return theBridge.write(buffer);
		}
	}

	class SourceChannel extends Bridge.SourceChannel {
		private TransparentBridge theBridge;

		SourceChannel(TransparentBridge bridge) {
			theBridge = bridge;
		}
		
		public int read(org.joshvm.util.ByteBuffer buffer) {
			return theBridge.read(buffer);
		}
	}
}


