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

#include "javacall_defs.h"

static char* javacall_static_properties_application_md_keys[] = {
	"org.joshvm.ams.jams.localconn",
    "smartps.jdevfs.drivers.METER",
	"smartps.jdevfs.drivers.POWER_D",
	"smartps.jdevfs.drivers.PULSE_CH",
	"smartps.jdevfs.drivers.UL_URT",
	"smartps.jdevfs.drivers.EXT1_URT",
	"smartps.jdevfs.drivers.BLE_URT",
	NULL
};

static char* javacall_static_properties_application_md_values[] = {
	"socket://127.0.0.1:9900",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
	"com.tengineer.smartps.jdevfs.DevfsNetworkEMUHandler",
    NULL
};


static char* javacall_static_properties_internal_md_keys[] = {
	"logging.level.channel.FC",
    NULL
};


static char* javacall_static_properties_internal_md_values[] = {
	"0",
    NULL
};

char** javacall_static_properties_md_keys[] = {
    javacall_static_properties_application_md_keys,
    javacall_static_properties_internal_md_keys,
    NULL
};

char** javacall_static_properties_md_values[] = {
    javacall_static_properties_application_md_values,
    javacall_static_properties_internal_md_values,
    NULL
};

