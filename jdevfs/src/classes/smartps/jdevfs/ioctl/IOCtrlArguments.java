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

package smartps.jdevfs.ioctl;

import org.joshvm.util.ByteBuffer;
import smartps.jdevfs.ioctl.arguments.NullArgument;

public abstract class IOCtrlArguments {
	protected ByteBuffer argbuf;

	private final static byte TYPE_BYTE = 0;
	private final static byte TYPE_INT = 1;
	private final static byte TYPE_BUF = 2;

	private final static NullArgument nullArgument;

	static {
		nullArgument = new NullArgument();
	}
	
	protected IOCtrlArguments(int size) {
		argbuf = ByteBuffer.allocateDirect(size);
	}
	
	public final static IOCtrlArguments create(String type) {
		try {
			if (type == null) {
				return nullArgument;
			}
			return (IOCtrlArguments)Class.forName("smartps.jdevfs.ioctl.arguments."+type).newInstance();
		} catch (InstantiationException ie) {
			throw new IllegalArgumentException("IOCtrlArguments instant can't be created: "+type);
		} catch (ClassNotFoundException cne) {
			throw new IllegalArgumentException("No such IOCtrlArguments class is found: "+type);
		} catch (IllegalAccessException iae) {
			throw new IllegalArgumentException(iae.toString());
		}
	}
	
	protected void putInt(int arg) {
		argbuf.putInt(arg);
	}
	protected void putByte(byte arg) {
		argbuf.put(arg);
	}
	protected void putByteBuffer(ByteBuffer arg) {
		argbuf.put(arg);
	}
	
	protected int getInt() {
		return argbuf.getInt();
	}

	protected byte getByte() {
		return argbuf.get();
	}

	protected ByteBuffer getByteBuffer(int size) {
		ByteBuffer result = argbuf.slice();
		result.limit(size);
		return result;
	}
	
	public byte[] asByteArray() {
		return argbuf.array();
	}

	public abstract void updateFromByteBuffer();
}

