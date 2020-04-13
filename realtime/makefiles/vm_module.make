# Copyright (C) Max Mu
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Please visit www.joshvm.org if you need additional information or
# have any questions.
REALTIME_SRC_DIR = $(REALTIME_DIR)/src

JSR_EXT_INCLUDE_DIRS +=

ROMGEN_CFG_FILES += ${REALTIME_SRC_DIR}/config/realtime_rom.cfg

ifeq ($(compiler), visCPP)
REALTIME_Obj_Files = \
	realtime_kni.obj

realtime_kni.obj: $(REALTIME_SRC_DIR)/native/realtime_kni.c
	$(BUILD_C_TARGET_NO_PCH)

else

REALTIME_Obj_Files = \
	realtime_kni.o

realtime_kni.o: $(REALTIME_SRC_DIR)/native/realtime_kni.c
	$(BUILD_C_TARGET)

endif