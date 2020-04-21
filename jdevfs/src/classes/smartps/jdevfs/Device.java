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

	/**
	 * Constructor of Device with specified driver's class name.
	 *
     * @param driverName currently uses null as most case
     * @throww DriverNotFoundException if the driverName specified driver is not found
     */
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

	/**
	 * Open device.
     * @param devName the name of device, e.g. "/dev/METER"
     * @param mode Device.READ_ONLY, Device.WRITE_ONLY, Device.READ_WRITE
     * @throws IOException if any error occurs.
     * @throww DriverNotFoundException if devName is not found
     */
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

	
	/**
	 * Close the device
	 * @throws IOException if any error occurs.
	 */
	public void close() throws IOException {
		filehandle.close();
	}

	
	/**
	 * Send ioctl command to the dev file.
	 * @param cmd is the command to send to ioctl
	 * @param arg is the argument to ioctl. If there's no argument for
	 *		  the operation, use NullArgument which is created by
	 *		  IOCtrlArguments.create(null)
	 * @return length of data really written
	 * @throws IOException if any error occurs.
	 */
	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException {
		filehandle.ioctl(cmd, arg);
	}

	
	/**
	 * Check if there's any incoming data package available. If there's
	 * not any available, wait for timeout milliseconds
	 *
	 * @return true if available, false not
	 * @throws IOException if any error occurs.
	 */
	public boolean poll(int timeout) throws IOException {
		return filehandle.poll(timeout);
	}

	
	/**
	 * Check if there's any incoming data package available
	 *
	 * @return true if available, false not
	 * @throws IOException if any error occurs.
	 */
	public boolean poll() throws IOException {
		return filehandle.poll();
	}

	
	/**
	 * Move current file access position
	 * @param offset the position to seek
	 * @param whence SEEK_SET: offset from beginning; 
	 *				 SEEK_CUR: to current position; 
	 *				 SEEK_END: offset from the end
	 * @return the position after seeking
	 * @throws IOException if any error occurs.
	 */
	public long lseek(long offset, int whence) throws IOException {
		return filehandle.lseek(offset, whence);
	}
}


