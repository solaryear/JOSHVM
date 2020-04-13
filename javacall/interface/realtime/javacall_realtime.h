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

#ifndef __JAVACALL_REALTIME_H
#define __JAVACALL_REALTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_defs.h"

#define JAVACALL_REALTIME_MAX_NAME_LENGTH 16

typedef struct {
    char name[JAVACALL_REALTIME_MAX_NAME_LENGTH]; // '\0' terminated string
} javacall_realtime_event;

typedef struct {
    javacall_int64 time;
    int index;
    javacall_realtime_event event;
} javacall_realtime_event_ex;

javacall_result javacall_realtime_event_add(javacall_realtime_event* event);

javacall_result javacall_realtime_event_get(javacall_int64 last_time,
    int last_index, javacall_realtime_event_ex* event);

void javanotify_realtime_event(const javacall_handle handle);

#ifdef __cplusplus
}
#endif

#endif
