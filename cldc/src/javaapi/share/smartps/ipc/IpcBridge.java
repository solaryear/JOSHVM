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

import org.joshvm.util.ByteBuffer;

class IpcBridge extends Bridge {
	
	private Bridge.SinkChannel sinkChannel;
	private Bridge.SourceChannel sourceChannel;

	private final static int DEFAULT_NATIVE_BUFFER_SIZE = 1024;
	private int bufferSize;
	private final int bridgeNativeId;
	
	IpcBridge(String bridgeName) {
		this(bridgeName, DEFAULT_NATIVE_BUFFER_SIZE);
	}

	IpcBridge(String bridgeName, int minBufferSize) {		
		bridgeNativeId = getNativeBridgeId0(bridgeName, minBufferSize);
		bufferSize = minBufferSize;
		if (bridgeNativeId < 0) {
			throw new OutOfMemoryError();
		}
	}

	public synchronized Bridge.SinkChannel sink() {
		if (sinkChannel == null) {
			sinkChannel = new IpcBridge.SinkChannel(this);
		}
		return sinkChannel;
	}

	public synchronized Bridge.SourceChannel source() {
		if (sourceChannel == null) {
			sourceChannel = new IpcBridge.SourceChannel(this);
		}
		return sourceChannel;
	}

	protected int write(org.joshvm.util.ByteBuffer buffer) {
		byte[] transferBuffer;
		
		int size = buffer.remaining();
		if (size > bufferSize) {
			size = bufferSize;
		}

		transferBuffer = new byte[size];
		buffer.get(transferBuffer, 0, size);
		
		return write0(bridgeNativeId, transferBuffer, 0, transferBuffer.length);
	}

	protected int read(org.joshvm.util.ByteBuffer buffer) {
		byte[] transferBuffer;
		
		int size = buffer.remaining();
		if (size > bufferSize) {
			size = bufferSize;
		}
		transferBuffer = new byte[size];
		
		int readLen = read0(bridgeNativeId, transferBuffer, 0, transferBuffer.length);
		
		buffer.put(new ByteBuffer(transferBuffer, 0, 0, readLen));
		
		return readLen;
	}

	
	public int getBufferSize() {
		return bufferSize;
	}

	private native int getNativeBridgeId0(String bridgeName, int minBufferSize);
	private native int read0(int bridgeId, byte[] bufferForReading, int offset, int bytesToRead);
	private native int write0(int bridgeId, byte[] bufferForWritting, int offset, int bytesToWrite);

	class SinkChannel extends Bridge.SinkChannel {
		private IpcBridge theBridge;
		
		SinkChannel(IpcBridge bridge) {
			theBridge = bridge;
		}
		
		public int write(org.joshvm.util.ByteBuffer buffer) {
			return theBridge.write(buffer);
		}
	}

	class SourceChannel extends Bridge.SourceChannel {
		private IpcBridge theBridge;

		SourceChannel(IpcBridge bridge) {
			theBridge = bridge;
		}
		
		public int read(org.joshvm.util.ByteBuffer buffer) {
			return theBridge.read(buffer);
		}
	}
}

