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

package com.sun.cldc.io.j2me.file;

import java.io.IOException;
import smartps.jdevfs.Device;
import smartps.jdevfs.DevfsOpsHandler;
import smartps.jdevfs.ioctl.IOCtrlArguments;

public class DevfsDefaultHandler implements DevfsOpsHandler {
	//private SecurityToken securityToken;
	private DefaultFileHandler filehandle;
	public static final int INVALID_FILE_HANDLE = -1;
	private int handle;

	public DevfsDefaultHandler() {
		filehandle = new DefaultFileHandler();
		handle = INVALID_FILE_HANDLE;
	}
	
	/*
	public DevfsFileHandler(SecurityToken token) {
		securityToken = token;
		filehandle = new DefaultFileHandler();
		handle = INVALID_FILE_HANDLE;
	}
	*/
	
	final public void open(String devName, int mode) throws IOException {
		//securityToken.checkIfPermissionAllowed(Permissions.AMS);
		filehandle.connect("dev/", devName);

		switch (mode) {
		case Device.READ_ONLY:
			filehandle.openForRead();
			handle = filehandle.getReadHandle();
			break;
		case Device.WRITE_ONLY:
			filehandle.openForWrite();
			handle = filehandle.getWriteHandle();
			break;
		case Device.READ_WRITE:
			filehandle.openForReadAndWrite();
			handle = filehandle.getReadHandle();
			break;
		default:
			throw new IllegalArgumentException();
		}
	}

	/**
     * Reads data from the file to an array.
     * @param b array for input data
     * @param off index in the input array
     * @param len length of data to read
     * @return length of data really read
     * @throws IOException if any error occurs.
     */
    public int read(byte b[], int off, int len) throws IOException {
		return filehandle.read(b, off, len);
	}
	
	/**
     * Write data from an array to the file.
     * @param b array of output data
     * @param off index in the output array
     * @param len length of data to write
     * @return length of data really written
     * @throws IOException if any error occurs.
     */
    public int write(byte b[], int off, int len) throws IOException {
		return filehandle.write(b, off, len);
	}

	public void close() throws IOException {
		filehandle.close();
	}
	
	final protected int fileHandle() {
		return handle;
	}

	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException {
		checkOpen();
		ioctl0(handle, cmd, arg.asByteArray());
	}
	
	public boolean poll(int timeout) throws IOException {
		checkOpen();
		throw new IOException("Timeout is currently not supported");
	}

	public boolean poll() throws IOException {
		checkOpen();
		return poll0(handle);
	}
	
	public long lseek(long offset, int whence) throws IOException {
		checkOpen();
		return lseek0(handle, offset, whence);
	}

	final protected native void ioctl0(int handle, int cmd, byte[] arg) throws IOException;

	final protected native boolean poll0(int handle) throws IOException;
	
	final protected native long lseek0(int handle, long offset, int whence) throws IOException;

	private void checkOpen() throws IOException {
		if (handle == INVALID_FILE_HANDLE) {
			throw new IOException("Device not open");
		}
	}
}

