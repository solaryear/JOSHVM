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
import javax.microedition.io.*;
import java.io.*;
import smartps.jdevfs.ioctl.IOCtrlArguments;

public class DevfsNonBlockingHandler implements smartps.jdevfs.DevfsOpsHandler {
	private int interval = 500;
	private InputStream in;
	private OutputStream out;
	private StreamConnection connection;
	
	public int read(byte b[], int off, int len) throws IOException {
		System.out.println("Read from EMU");
		return in.read(b, off, len);
	}

	public void setCheckingInterval(int ms) {
		interval = ms;
	}

	public void open(String devName, int mode) throws IOException {
		connection = (StreamConnection)Connector.open("socket://127.0.0.1:9977");
		System.out.println("Open Input...");
		in = connection.openInputStream();
		System.out.println("Open Output...");
		out = connection.openOutputStream();
		System.out.println("Connected to emu");
	}
	public int write(byte b[], int off, int len) throws IOException {
		System.out.println("Write to EMU");
		out.write(b, off, len);
		return len;
	}
	public void close() throws IOException {
		System.out.println("Closing connection");
		in.close();
		out.close();
		connection.close();
	}
	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException {

	}
	
	public boolean poll(int timeout) throws IOException {
		throw new IOException("Not supported");
	}
	public boolean poll() throws IOException {
		return in.available() > 0?true:false;
	}
	
	public long lseek(long offset, int whence) throws IOException {
		throw new IOException("Not support lseek");
	}
}

