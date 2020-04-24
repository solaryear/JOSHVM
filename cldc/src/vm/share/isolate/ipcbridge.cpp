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

#include "incls/_precompiled.incl"
#include "incls/_ipcbridge.cpp.incl"

extern "C" {

#if ENABLE_PCSL
#include <pcsl_string.h>
#include <pcsl_memory.h>

const static int NUMBER_NATIVE_BRIDGE_BUFFER = 64;
static struct _bridgeTable {
	pcsl_string name;
	int available;
	int capacity;
	jbyte* buffer;
} bridgeTable[NUMBER_NATIVE_BRIDGE_BUFFER] = {{NULL, 0, 0}};

static int ipcBufferSize = 0;

#endif

#if ENABLE_PCSL
static int getIPCBridgeIdByName(pcsl_string* name) {

	jint result;
	
	for (int i = 0 ; i < ipcBufferSize; i++) {
		if (pcsl_string_compare(name, &(bridgeTable[i].name), &result) == PCSL_STRING_OK) {
			if (result == 0) {
				return i;
			}
		}
	}

	return -1;
}
#endif

static jbyte* createIPCBridgeBuffer(int size) {
#if ENABLE_PCSL
	return (jbyte*)pcsl_mem_malloc(size);
#else
	return NULL;
#endif
}

static jbyte* getIPCBridgeBuffer(int id) {
#if ENABLE_PCSL
	if (id < 0 || id >= ipcBufferSize) {
		return NULL;
	}

	return bridgeTable[id].buffer;
#else
	return NULL;
#endif
}

static int getIPCBridgeBufferAvailable(int id) {
#if ENABLE_PCSL

	if (id < 0 || id >= ipcBufferSize) {
		return NULL;
	}

	return bridgeTable[id].available;
#else
	return -1;
#endif
}

static int getIPCBridgeBufferCapacity(int id) {
#if ENABLE_PCSL
	if (id < 0 || id >= ipcBufferSize) {
		return NULL;
	}

	return bridgeTable[id].capacity;
#else
	return -1;
#endif
}

#if ENABLE_PCSL
static int registerIPCBuffer(pcsl_string* pstrName, jbyte* ipcBuffer, int capacity) {
	int id = -1;
	
	if (ipcBufferSize < NUMBER_NATIVE_BRIDGE_BUFFER) {
		if (PCSL_STRING_OK == pcsl_string_dup(pstrName,&(bridgeTable[ipcBufferSize].name))) {
			bridgeTable[ipcBufferSize].available = 0;
			bridgeTable[ipcBufferSize].capacity = capacity;
			bridgeTable[ipcBufferSize].buffer = ipcBuffer;
			id = ipcBufferSize++;
		}
	}

	return id;
}
#endif

static void setIPCBridgeBufferAvailable(int bridgeId, int availableSize) {
#if ENABLE_PCSL
	if (bridgeId < 0 || bridgeId >= ipcBufferSize) {
		return;
	}

	bridgeTable[bridgeId].available = availableSize;
#endif
}

#if ENABLE_PCSL
/**
 * Create pcsl_string from the specified Java String object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * @param java_str pointer to the Java String instance
 * @param pcsl_str pointer to the pcsl_string instance
 * @return status of the operation
 */
static pcsl_string_status jstring_to_pcsl_string(jstring java_str,
					       pcsl_string * pcsl_str) {
  if (pcsl_str == NULL) {
    return PCSL_STRING_EINVAL;
  }

  if (KNI_IsNullHandle(java_str)) {
    * pcsl_str = PCSL_STRING_NULL;
    return PCSL_STRING_OK;
  } else {
    const jsize length  = KNI_GetStringLength(java_str);

    if (length < 0) {
      * pcsl_str = PCSL_STRING_NULL;
      return PCSL_STRING_ERR;
    } else if (length == 0) {
      * pcsl_str = PCSL_STRING_EMPTY;
      return PCSL_STRING_OK;
    } else {
      jchar * buffer = (jchar*)pcsl_mem_malloc(length * sizeof(jchar));

      if (buffer == NULL) {
	    * pcsl_str = PCSL_STRING_NULL;
	    return PCSL_STRING_ENOMEM;
      }

      KNI_GetStringRegion(java_str, 0, length, buffer);

      {
	    pcsl_string_status status =
	    pcsl_string_convert_from_utf16(buffer, length, pcsl_str);

	    pcsl_mem_free(buffer);

	    return status;
      }
    }
  }
}

/**
 * The method gets 'pcsl_string' from 'Java String'.
 *
 * Note: the caller is responsible for calling
 * 'pcsl_string_free(*string);
 *  pcsl_mem_free(*string);'
 * after use.
 *
 * @param stringHandle The input 'Java String' handle
 * @param string The output 'pcsl_string'
 * @return  0 if conversion passed successfully,
 *         -1 if any error occurred.
 */
static int get_pcsl_string(jobject stringHandle, pcsl_string ** string)
{

    if (KNI_IsNullHandle(stringHandle)) {
        KNI_ThrowNew(KNINullPointerException, "Try to convert null string");
        return -1;
    } else {
		pcsl_string * str = (pcsl_string *)pcsl_mem_malloc(sizeof(pcsl_string));    
	        
		if (str == NULL) {
		    KNI_ThrowNew(KNIOutOfMemoryError, NULL);
		    return -1;
		}

	 	if (jstring_to_pcsl_string(stringHandle, str) != PCSL_STRING_OK) {
		    pcsl_mem_free(str);
			return -1;
		}

		* string = str;	
    }
    return 0;
}
#endif

//IPC Bridge KNI implementations from here
jint Java_smartps_ipc_IpcBridge_getNativeBridgeId0() {
  int id = -1;
 
#if ENABLE_PCSL
  const jint requiredBufferSize = KNI_GetParameterAsInt(2);
  jbyte *ipcBuffer;
  int bufferize;

  pcsl_string *pstrBridgeName = NULL;	

  KNI_StartHandles(1);
  KNI_DeclareHandle(stringBridgeName);
	
  KNI_GetParameterAsObject(1, stringBridgeName);

  if (requiredBufferSize >= 0 && !get_pcsl_string(stringBridgeName, &pstrBridgeName)) {

	
	id = getIPCBridgeIdByName(pstrBridgeName);
	if (id < 0) {
		if (NULL != (ipcBuffer = createIPCBridgeBuffer(requiredBufferSize))) {
			id = registerIPCBuffer(pstrBridgeName, ipcBuffer, requiredBufferSize);
		}
	} else {
		bufferize = getIPCBridgeBufferCapacity(id);
		if (bufferize < requiredBufferSize) {
			id = -1;
		}
	}	
  }

  if (pstrBridgeName) {
	pcsl_string_free(pstrBridgeName);
  }
	
  KNI_EndHandles();
 #endif
  KNI_ReturnInt(id);
}

jint Java_smartps_ipc_IpcBridge_read0() {
  jint id = KNI_GetParameterAsInt(1);
  jint offset = KNI_GetParameterAsInt(3);
  jint length = KNI_GetParameterAsInt(4);
  jbyte* ipcBuffer = getIPCBridgeBuffer(id);
  int availableBytes = getIPCBridgeBufferAvailable(id);
  
  KNI_StartHandles(1);
  KNI_DeclareHandle(byteArrHandle);
  KNI_GetParameterAsObject(2, byteArrHandle);
  
  if (ipcBuffer != NULL && availableBytes > 0) {
  	if (length > availableBytes) {
		length = availableBytes;
	}
	
	KNI_SetRawArrayRegion(byteArrHandle, offset * sizeof(jbyte),
                                      length * sizeof(jbyte), ipcBuffer);
	setIPCBridgeBufferAvailable(id, 0);
  } else {
	if (ipcBuffer == NULL) {
		KNI_ThrowNew(KNIIOException, "IPC buffer not found");
	} else {
		length = 0;
	}
  }

  KNI_EndHandles();
  KNI_ReturnInt(length);
}

jint Java_smartps_ipc_IpcBridge_write0() {
  jint id = KNI_GetParameterAsInt(1);
  jint offset = KNI_GetParameterAsInt(3);
  jint length = KNI_GetParameterAsInt(4);
  jbyte* ipcBuffer = getIPCBridgeBuffer(id);
  int size = getIPCBridgeBufferCapacity(id);
  int availableBytes = getIPCBridgeBufferAvailable(id);

  KNI_StartHandles(1);
  KNI_DeclareHandle(byteArrHandle);
  KNI_GetParameterAsObject(2, byteArrHandle);
  
  if (ipcBuffer != NULL && availableBytes == 0) {
	if (length > size) {
		KNI_ThrowNew(KNIIOException, "Message too long to fit IPC buffer");
	} else {
		KNI_GetRawArrayRegion(byteArrHandle, offset * sizeof(jbyte),
                                      length * sizeof(jbyte), ipcBuffer);
		setIPCBridgeBufferAvailable(id, length);
	}
  } else {
	if (ipcBuffer == NULL) {
		KNI_ThrowNew(KNIIOException, "IPC buffer not found");
	} else {
		//busy
		length = 0;
	}
  }

  KNI_EndHandles();
  KNI_ReturnInt(length);
}

} // extern "C"


