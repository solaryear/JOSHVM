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
#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_realtime.h"
#include "javacall_time.h"
#include "javacall_logging.h"

#define JC_MAX_REALTIME_EVENTS 15

static javacall_realtime_event_ex events[JC_MAX_REALTIME_EVENTS] = {0};
static int cur_index = 0;
static int block_flag = 0;

static void realtime_lock() {
}

static void realtime_unlock() {
}

javacall_result javacall_realtime_event_add(javacall_realtime_event* event) {
    if (event == NULL) {
        return JAVACALL_FAIL;
    }
    int i = 0;
    for (; i < JAVACALL_REALTIME_MAX_NAME_LENGTH; i++) {
        if (event->name[i] == 0) {
            break;
        }
    }
    if (i == 0 || i >= JAVACALL_REALTIME_MAX_NAME_LENGTH) {
        return JAVACALL_FAIL;
    }
    realtime_lock();
    int index = cur_index % JC_MAX_REALTIME_EVENTS;
    events[index].time = javacall_time_get_milliseconds_since_1970();
    events[index].index = cur_index;
    events[index].event = *event;
    cur_index++;
    realtime_unlock();
    if (block_flag) {
        block_flag = 0;
        javanotify_realtime_event(-1);
    }
    return JAVACALL_OK;
}

javacall_result javacall_realtime_event_get(javacall_int64 last_time,
    int last_index, javacall_realtime_event_ex* event) {

    javacall_result result = JAVACALL_FAIL;
    if (event == NULL) {
        return JAVACALL_FAIL;
    }

    realtime_lock();
    int found_index = -1;
    for (int index = 0; index < JC_MAX_REALTIME_EVENTS; index++) {
        if (events[index].time >= last_time &&
            events[index].index > last_index) {
            if (found_index == -1 ||
                events[index].index < events[found_index].index) {
                found_index = index;
            }
        }
    }
    if (found_index == -1) {
        block_flag = 1;
        result = JAVACALL_WOULD_BLOCK;
    } else {
        *event = events[found_index];
        result = JAVACALL_OK;
    }
    realtime_unlock();
    return result;
}

#ifdef __cplusplus
}
#endif
