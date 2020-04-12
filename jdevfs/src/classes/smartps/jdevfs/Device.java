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

package smartps.jdevfs;

import com.sun.cldc.io.j2me.file.*;
import java.io.IOException;
import org.joshvm.security.internal.SecurityToken;
import smartps.jdevfs.ioctl.IOCtrlArguments;
	
public class Device {
	public final static int READ_ONLY = 0;
	public final static int WRITE_ONLY = 1;
	public final static int READ_WRITE = 2;

	public final static int SEEK_SET = 0;
	public final static int SEEK_CUR = 1;
	public final static int SEEK_END = 2;
	
	
	private DevfsOpsHandler filehandle;

	public Device(String driverName) throws DriverNotFoundException {
		filehandle = loadDriver(driverName);
	}

	private DevfsOpsHandler loadDriver(String driverName) throws DriverNotFoundException {
		String reason;
		
		try {
			if (driverName != null) {
				return (DevfsOpsHandler) (Class.forName(driverName)).newInstance();
			} else {
				return null;
			}
		} catch (ClassNotFoundException e) {
			reason = e.toString();
		} catch (Error e) {
			reason = e.toString();
		} catch (IllegalAccessException e) {
			reason = e.toString();
		} catch (InstantiationException e) {
			reason = e.toString();
		}
			
		throw new DriverNotFoundException(reason);
	}
	
	public void open(String devName, int mode) throws IOException {
		try {
			if (filehandle == null) {
				String device = devName.substring(5); //skip "/dev/"
				String driverName = System.getProperty("smartps.jdevfs.drivers."+device);
				if (driverName == null) {
					System.out.println("loading driver for "+devName+" : default");
					filehandle = new DevfsDefaultHandler();
				} else {
					System.out.println("loading driver for "+devName+" : "+driverName);
					filehandle = loadDriver(driverName);
				}
			}
			filehandle.open(devName, mode);
		} catch (StringIndexOutOfBoundsException e) {
			throw new DriverNotFoundException(e.toString());
		} catch (DeviceNotSupportException dnse) {
			throw new DriverNotFoundException(dnse.toString());
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

	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException {
		filehandle.ioctl(cmd, arg);
	}
	
	public boolean poll(int timeout) throws IOException {
		return filehandle.poll(timeout);
	}

	public boolean poll() throws IOException {
		return filehandle.poll();
	}
	
	public long lseek(long offset, int whence) throws IOException {
		return filehandle.lseek(offset, whence);
	}
}


