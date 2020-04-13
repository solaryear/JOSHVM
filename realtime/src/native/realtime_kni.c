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
#include <kni.h>
#include <kni_globals.h>
#include <sni.h>
#include <sni_event.h>

#if ENABLE_PCSL
#include "javacall_realtime.h"
#endif

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_realtime_RTEventController_getRTEvent0()
{
    int index = -1;
#if ENABLE_PCSL
    jfieldID name_field_id;
    javacall_realtime_event_ex event_ex;
    javacall_int64 last_time;
    int last_index;
    SNIReentryData* info;
    javacall_result result;

    KNI_StartHandles(3);
    KNI_DeclareHandle(eventObj);
    KNI_DeclareHandle(classObj);
    KNI_DeclareHandle(nameStr);

    KNI_GetParameterAsObject(1, eventObj);
    KNI_GetObjectClass(eventObj, classObj);
    last_time = (javacall_int64)KNI_GetParameterAsLong(2);
    last_index = (int)KNI_GetParameterAsInt(4);

    info = (SNIReentryData*)SNI_GetReentryData(NULL);
    if (info == NULL || info->status == 0) {
        result = javacall_realtime_event_get(last_time, last_index, &event_ex);
        if (result == JAVACALL_WOULD_BLOCK) {
            SNIEVT_wait(RT_EVENT_SIGNAL, -1, NULL);
        } else if (result == JAVACALL_OK) {
            name_field_id = KNI_GetFieldID(classObj, "name", "Ljava/lang/String;");
            if (name_field_id) {
                KNI_NewStringUTF(event_ex.event.name, nameStr);
                KNI_SetObjectField(eventObj, name_field_id, nameStr);
            }
            index = event_ex.index;
        }
    }
    KNI_EndHandles();
#endif

    KNI_ReturnInt(index);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_realtime_RTEventController_interruptGetRTEvent0()
{
#if ENABLE_PCSL
    SNIEVT_signal(RT_EVENT_SIGNAL, -1, -1);
#endif

    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_realtime_RTEventController_fireEvent0()
{
#if ENABLE_PCSL
    int offset, len;
    javacall_realtime_event event = {0};

    KNI_StartHandles(1);
    KNI_DeclareHandle(dataObj);

    KNI_GetParameterAsObject(1, dataObj);
    offset = (int)KNI_GetParameterAsInt(2);
    len = (int)KNI_GetParameterAsInt(3);

    if (len > JAVACALL_REALTIME_MAX_NAME_LENGTH - 1) {
        len = JAVACALL_REALTIME_MAX_NAME_LENGTH - 1;
    }

    SNI_BEGIN_RAW_POINTERS
    char* dataPtr = (char*)SNI_GetRawArrayPointer(dataObj)+offset;
    for (int i = 0; i < len; i++) {
        event.name[i] = dataPtr[i];
    }
    SNI_END_RAW_POINTERS

    javacall_realtime_event_add(&event);

    KNI_EndHandles();
#endif

    KNI_ReturnVoid();
}
