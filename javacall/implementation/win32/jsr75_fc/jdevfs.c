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
#include "javacall_jdevfs.h"
#include "javacall_logging.h"
#include "javacall_socket.h"
#include "javacall_file.h"

#include <string.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

typedef struct _tFDItem {
	wchar_t* devname;
	char* fname;
}tFDItem;

static tFDItem fdtable[] = {
		{L"/dev/ON_FL", "C:\\java\\ON_FL.dev"},
		{L"/dev/EXT_FL", "C:\\java\\EXT_FL.dev"},
};

static int initialized = 0;

static javacall_result getDeviceByDevName(wchar_t* wfn, tFDItem* item) {
	for (int i = 0; i < sizeof(fdtable)/sizeof(tFDItem); i++) {
		if (!wcscmp(fdtable[i].devname, wfn)) {
			memcpy(item, &fdtable[i], sizeof(tFDItem));
			return JAVACALL_OK;
		}
	}
	return JAVACALL_FAIL;
}

javacall_result javacall_jdevfs_open(const javacall_utf16*  unicodeFileName, 
                                   int                      fileNameLen, 
                                   int                      flags,
                                   javacall_handle* /* OUT */ handle) {
    wchar_t wOsFilename[MAX_NULL_TERMINATED_DEVFS_PATH_LENGTH]; // max file name
    tFDItem item;
    int fd;
	
	if( fileNameLen > JAVACALL_MAX_DEVFS_PATH_LENGTH ) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_FILE, "javacall_jdevfs_open: Device name length exceeds limit");
		return JAVACALL_FAIL;
	}
	
	memcpy(wOsFilename, unicodeFileName, fileNameLen*sizeof(wchar_t));
	wOsFilename[fileNameLen] = 0;
	fwprintf(stderr, L"javacall_jdevfs_open: %s\n", wOsFilename);

	javacall_handle h;
	if (JAVACALL_OK != getDeviceByDevName(wOsFilename, &item)) {
		printf("dev not found\n");
		return JAVACALL_FAIL;
	}

	{
		fprintf(stderr, "opening as regular file...");
		//Should be open as regular file
		char* fn = item.fname;
		if (fn == NULL) {
			fprintf(stderr, "failed\n");
			return JAVACALL_FAIL;
		}

		fprintf(stderr, " %s\n", fn);
		
	    fd = _open(fn, O_RDWR | O_CREAT, _S_IREAD | _S_IWRITE);

	    if (fd == -1) {
	        *handle = NULL;
	        return JAVACALL_FAIL;
	    }

	    *handle = (void *)fd;
	    return JAVACALL_OK;
	}

	*handle = h;
	return JAVACALL_OK;
}

javacall_result javacall_jdevfs_poll(javacall_handle handle, javacall_bool* retValue) {
	return JAVACALL_FAIL;
}

javacall_result javacall_jdevfs_ioctl_start(javacall_handle handle, javacall_int64 cmd, void* arg, javacall_int32 len) {
	return JAVACALL_FAIL;
}

javacall_result javacall_jdevfs_ioctl_finish(javacall_handle handle, javacall_int64 cmd, void* arg, javacall_int32 len) {
	return JAVACALL_FAIL;
}

javacall_int64 javacall_jdevfs_seek(javacall_handle handle, 
                        javacall_int64 offset, 
                        javacall_file_seek_flags flag) {
	if (handle < 0) {
		return JAVACALL_FAIL;
	} else {
		return javacall_file_seek(handle, offset, flag);
	}
}


