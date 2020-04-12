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

public abstract class IOCtrlArguments {
	private ByteBuffer argbuf;

	private final static byte TYPE_BYTE = 0;
	private final static byte TYPE_INT = 1;
	private final static byte TYPE_BUF = 2;
	
	protected IOCtrlArguments(int size) {
		argbuf = ByteBuffer.allocateDirect(size);
	}
	
	public static IOCtrlArguments create(String type) {
		try {
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
		argbuf.put(TYPE_INT);
		argbuf.putInt(arg);
	}
	protected void putByte(byte arg) {
		argbuf.put(TYPE_BYTE);
		argbuf.put(arg);
	}
	protected void putByteBuffer(ByteBuffer arg) {
		argbuf.put(TYPE_BUF);
		argbuf.put((byte)(arg.remaining()&0xff));
		argbuf.put(arg);
	}
	
	protected Object get() throws NumberFormatException {
		byte type = argbuf.get();
		
		if (type == TYPE_INT) {
			return new Integer(argbuf.getInt());
		}
		if (type == TYPE_BYTE) {
			return new Byte(argbuf.get());
		}
		if (type == TYPE_BUF) {
			byte len = argbuf.get();
			byte[] buf = new byte[len&0xff];
			argbuf.get(buf);
			return new ByteBuffer(buf);
		}

		throw new NumberFormatException("Unknown type");
	}
	
	public byte[] asByteArray() {
		return argbuf.array();
	}

	public abstract void updateFromByteBuffer();
}

