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
#ifndef __JAVACALL_JDEVFS_H
#define __JAVACALL_JDEVFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_defs.h"
#include "javacall_file.h"

#define JAVACALL_MAX_DEVFS_PATH_LENGTH 16
#define MAX_NULL_TERMINATED_DEVFS_PATH_LENGTH (JAVACALL_MAX_DEVFS_PATH_LENGTH+1)

javacall_result javacall_jdevfs_poll(javacall_handle handle, javacall_bool* retValue);
javacall_result javacall_jdevfs_ioctl_start(javacall_handle handle, javacall_int64 cmd, void* arg, javacall_int32 len);
javacall_result javacall_jdevfs_ioctl_finish(javacall_handle handle, javacall_int64 cmd, void* arg, javacall_int32 len);
javacall_int64 javacall_jdevfs_seek(javacall_handle handle, 
                        javacall_int64 offset, 
                        javacall_file_seek_flags flag); 
javacall_result javacall_jdevfs_open(const javacall_utf16*  unicodeFileName, 
                                   int                      fileNameLen, 
                                   int                      flags,
                                   javacall_handle* /* OUT */ handle);

#ifdef __cplusplus
}
#endif

#endif

