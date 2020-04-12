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
package com.tengineer.smartps.jdevfs;

import java.io.IOException;
import java.io.InterruptedIOException;
import javax.microedition.io.*;
import java.io.*;
import smartps.jdevfs.ioctl.IOCtrlArguments;
import smartps.jdevfs.DeviceNotSupportException;

public class DevfsNetworkEMUHandler implements smartps.jdevfs.DevfsOpsHandler {
	private final static int interval = 200; //poll() sleep interval in ms
	
	private InputStream in = null;
	private OutputStream out = null;
	private StreamConnection connection = null;
	private DataInputStream ioctlin = null;
	private DataOutputStream ioctlout = null;
	private StreamConnection ioctlconnection = null;

	protected void connect(String ip, int port) throws IOException {
		connection = (StreamConnection)Connector.open("socket://"+ip+":"+port);
		System.out.println("Open Input...");
		in = connection.openInputStream();
		System.out.println("Open Output...");
		out = connection.openOutputStream();
		System.out.println("Connected to emu port "+ port);
	}

	protected void ioctlconnect(String ip, int ioctlport) throws IOException {
		ioctlconnection = (StreamConnection)Connector.open("socket://"+ip+":"+ioctlport);
		System.out.println("Open IOCTLInput...");
		ioctlin = connection.openDataInputStream();
		System.out.println("Open IOCTLOutput...");
		ioctlout = connection.openDataOutputStream();
		System.out.println("Connected to emu ioctl port "+ioctlport);
	}
	
	public int read(byte b[], int off, int len) throws IOException {
		return in.read(b, off, len);
	}

	public void open(String devName, int mode) throws IOException, DeviceNotSupportException {
		int port,ioctlport;
		System.out.println("DevfsNetworkEMUHandler open..."+devName);
		if (devName.equals("/dev/METER")) {
			port = 9977;
			ioctlport = -1;
		} else if (devName.equals("/dev/POWER_D")) {
			port = 9979;
			ioctlport = -1;
		} else if (devName.equals("/dev/PULSE_CH")) {
			port = 9981;
			ioctlport = 9982;
		} else {
			throw new DeviceNotSupportException(devName);
		}
		
		connect("127.0.0.1", port);
		if (ioctlport > 0) {
			ioctlconnect("127.0.0.1", ioctlport);
		}
	}
	
	public int write(byte b[], int off, int len) throws IOException {
		out.write(b, off, len);
		return len;
	}
	
	public void close() throws IOException {
		System.out.println("Closing connection to EMU");
		if (in != null) {
			in.close();
			in = null;
		}
		if (out != null) {
			out.close();
			out = null;
		}
		if (connection != null) {
			connection.close();
			connection = null;
		}
	}
	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException {
		if (ioctlout != null) {
			ioctlout.writeInt(cmd);
			ioctlout.write(arg.asByteArray());
		}
		if (ioctlin != null) {
			int respcode = ioctlin.readInt();
			if (respcode < 0) {
				throw new IOException("ioctl operation failed: "+respcode);
			}
			int readlen = 0;
			while (respcode > readlen) {
				readlen += ioctlin.read(arg.asByteArray(), readlen, respcode - readlen);
			}
		}
	}
	
	public boolean poll(int timeout) throws IOException, InterruptedIOException  {
		boolean result = false;
		
		if (timeout == 0) {
			return poll();
		}
		
		long start = System.currentTimeMillis();
		long end = start + timeout;
		while (System.currentTimeMillis() < end) {
			if (poll()) return true;
			try {
				Thread.sleep(timeout > interval? interval: timeout);
			} catch (InterruptedException e) {
				throw new InterruptedIOException();
			}
		}
		return false;
	}
	
	public boolean poll() throws IOException {
		return in.available() > 0?true:false;
	}
	
	public long lseek(long offset, int whence) throws IOException {
		throw new IOException("Not support lseek");
	}
}

