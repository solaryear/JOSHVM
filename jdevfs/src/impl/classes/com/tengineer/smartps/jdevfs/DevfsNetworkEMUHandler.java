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
import com.sun.midp.log.LogChannels;
import com.sun.midp.log.Logging;

public class DevfsNetworkEMUHandler implements smartps.jdevfs.DevfsOpsHandler {
	private final static int interval = 200; //poll() sleep interval in ms
	
	private InputStream in = null;
	private OutputStream out = null;
	private StreamConnection connection = null;
	private DataInputStream ioctlin = null;
	private DataOutputStream ioctlout = null;
	private StreamConnection ioctlconnection = null;
	private boolean isPollSupported = false;

	protected void connect(String ip, int port) throws IOException {
		connection = (StreamConnection)Connector.open("socket://"+ip+":"+port);
		in = connection.openInputStream();
		out = connection.openOutputStream();
		Logging.report(Logging.INFORMATION, LogChannels.LC_FC, "Connected to emu port "+ port);
	}

	protected void ioctlconnect(String ip, int ioctlport) throws IOException {
		ioctlconnection = (StreamConnection)Connector.open("socket://"+ip+":"+ioctlport);
		ioctlin = ioctlconnection.openDataInputStream();
		ioctlout = ioctlconnection.openDataOutputStream();
		Logging.report(Logging.INFORMATION, LogChannels.LC_FC, "Connected to emu ioctl port "+ioctlport);
	}
	
	public int read(byte b[], int off, int len) throws IOException {
		if (in == null) {
			throw new IOException("Not supported operation");
		}
		return in.read(b, off, len);
	}

	public void open(String devName, int mode) throws IOException, DeviceNotSupportException {
		int port,ioctlport;
		
		Logging.report(Logging.INFORMATION, LogChannels.LC_FC, "DevfsNetworkEMUHandler open..."+devName);
		
		if (devName.equals("/dev/METER")) {
			port = 9977;
			ioctlport = -1;
		} else if (devName.equals("/dev/POWER_D")) {
			port = 9979;
			ioctlport = -1;
			isPollSupported = true;
		} else if (devName.equals("/dev/PULSE_CH")) {
			port = -1;
			ioctlport = 9982;
		} else if (devName.equals("/dev/UL_URT")) {
			port = 9983;
			ioctlport = -1;
			isPollSupported = true;
		} else if (devName.equals("/dev/EXT1_URT")) {
			port = 9985;
			ioctlport = -1;
			isPollSupported = true;
		} else if (devName.equals("/dev/BLE_URT")) {
			port = 9987;
			ioctlport = -1;
			isPollSupported = true;
		} else {
			throw new DeviceNotSupportException(devName);
		}
		
		if (port > 0) {
			connect("127.0.0.1", port);
		}
		
		if (ioctlport > 0) {
			ioctlconnect("127.0.0.1", ioctlport);
		}
	}
	
	public int write(byte b[], int off, int len) throws IOException {
		if (out == null) {
			throw new IOException("Not supported operation");
		}
		out.write(b, off, len);
		return len;
	}
	
	public void close() throws IOException {
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
		if (ioctlconnection == null) {
			throw new IOException("Not supported operation");
		}

		if (ioctlin != null) {
			int aval = ioctlin.available();
			if (aval > 0) {
				//purge all the data existing in ioctlin stream before sending ioctl command
				read(new byte[aval], 0, aval);
			}
		}
		
		if (ioctlout != null) {
			ioctlout.writeInt(cmd);
			ioctlout.write(arg.asByteArray());
		}
		if (ioctlin != null) {
			int respcode = ioctlin.readInt();
			if (respcode < 0) {
				throw new IOException("ioctl operation failed with respcode: "+respcode);
			}
			if (respcode > arg.asByteArray().length) {
				throw new IOException("ioctl operation failed with too long return message length: "+respcode);
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
		if (!isPollSupported || (in == null && ioctlin == null)) {
			throw new IOException("Not supported operation");
		}

		if (in != null) {
			if (in.available() > 0) {
				return true;
			}
		}

		if (ioctlin != null) {
			if (ioctlin.available() > 0) {
				return true;
			}
		}

		return false;
	}
	
	public long lseek(long offset, int whence) throws IOException {
		throw new IOException("Not support lseek");
	}
}

