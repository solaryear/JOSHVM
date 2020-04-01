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

import java.io.IOException;
import smartps.jdevfs.ioctl.IOCtrlArguments;

public interface DevfsOpsHandler {
	public void open(String devName, int mode) throws IOException;
	public int read(byte b[], int off, int len) throws IOException;
	public int write(byte b[], int off, int len) throws IOException;
	public void close() throws IOException;
	public void ioctl(int cmd, IOCtrlArguments arg) throws IOException;
	public boolean poll(int timeout) throws IOException;
	public boolean poll() throws IOException;
	public long lseek(long offset, int whence) throws IOException;
 }

