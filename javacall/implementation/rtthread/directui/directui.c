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

#include "javacall_directui.h"
#include <rtthread.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C"{
#endif
static int fd = -1;

#define DISPLAY_BUFFER_ROW       10   //row 行数 显示缓冲区 液晶是160*64个点数  每行显示8个点数组成一个字节8行可以表示64个点
#define DISPLAY_BUFFER_COLUMU    208  //column 列数 显示缓冲区 液晶是160*64个点数  每行显示8个点数组成一个字节 
#define LcdLightAdjustValueMax         100     //亮度最大值
#define LcdLightAdjustValueMin         0       //亮度最小值
#define LCD_FRAM_SIZE	DISPLAY_BUFFER_ROW*DISPLAY_BUFFER_COLUMU
#define LCD_CTRL_INIT_DEV					0    //液晶初始化 arg=ZX_NULL
#define	LCD_CTRL_REFRESH_SECTION  1		 //液晶某区域数据更新
#define LCD_CTRL_SET_SECTION			2    //液晶某区域全屏或者清屏
#define LCD_CTRL_GET_STATUS				3		 //液晶状态读取
#define LCD_CTRL_OPEN_DEV					4    //液晶唤醒打开（arg=ZX_NULL）
#define LCD_CTRL_CLOSE_DEV				5		 //液晶掉电关闭（arg=ZX_NULL）

const static rt_uint8_t BigGB_B5B1[48]=
{0x00,0x00,0x04,0x02,0x01,0x00,0x00,0x0F,0x00,0x00,0x00,0x01,0x06,0x00,0x00,0x00,
0x00,0x20,0x22,0x22,0xA2,0x22,0x22,0xE2,0x22,0x22,0xA2,0x22,0x3F,0x00,0x00,0x00,
0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xF0,0x00,0x00,0x00}; /*当*/

const static rt_uint8_t BigGB_C7B0[48]=
{0x01,0x01,0x01,0x09,0x07,0x01,0x01,0x01,0x01,0x01,0x03,0x0D,0x01,0x01,0x01,0x00,
0x00,0x00,0x7F,0x49,0x49,0x49,0x7F,0x00,0x00,0x3F,0x00,0x00,0x7F,0x00,0x00,0x00,
0x00,0x00,0xF0,0x00,0x20,0x10,0xE0,0x00,0x00,0x00,0x20,0x10,0xE0,0x00,0x00,0x00};	/*前*/

const static rt_uint8_t BigGB_D5FD[48]=
{0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x07,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,
0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0xFF,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x00,
0x20,0x20,0x20,0xE0,0x20,0x20,0x20,0xE0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00};	/*正*/

const static rt_uint8_t BigGB_CFF2[48]=
{0x00,0x01,0x01,0x01,0x03,0x05,0x09,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
0x00,0xFF,0x00,0x00,0x00,0x3F,0x21,0x21,0x21,0x3F,0x00,0x00,0x00,0xFF,0x00,0x00,
0x00,0xF0,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x20,0x10,0xE0,0x00,0x00};	/*向*/

const static rt_uint8_t BigGB_D3D0[48]=
{0x02,0x02,0x02,0x02,0x02,0x03,0x0E,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,
0x02,0x04,0x08,0x10,0x7F,0xC9,0x49,0x49,0x49,0x49,0x49,0x7F,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x20,0x10,0xE0,0x00,0x00,0x00,0x00}; /*有*/

const static rt_uint8_t BigGB_B9A6[48]=
{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xFF,0x01,0x01,0x01,0x80,0x87,0xF8,0x80,0x80,0x80,0xFF,0x00,0x00,
0x80,0xC0,0x80,0x80,0x00,0x10,0x20,0xC0,0x00,0x00,0x20,0x10,0x20,0xC0,0x00,0x00};	/*功*/

struct rt_lcd_zone_info
{
		rt_uint8_t		col_start;	/*数据刷新开始的列*/
		rt_uint8_t		col_end;		/*数据刷新结束的列*/
		rt_uint8_t		row_start;	/*数据刷新开始的页（行数/8）*/
		rt_uint8_t		row_end;		/*数据刷新结束的页（行数/8）*/
};

struct rt_lcd_refresh_param
{
		struct rt_lcd_zone_info zone_info; /*刷新位置*/
		rt_uint8_t 	*p_data;	 /*刷新的数据指针*/
};

struct rt_lcd_set_param
{
		struct rt_lcd_zone_info zone_info; /*刷新位置*/
		rt_uint8_t value;									 /*设置的数据值*/
};

javacall_result javacall_directui_init(void) {
	if (fd != -1) {
		return JAVACALL_OK;
	}
	
	fd = open("/dev/LCD", RT_DEVICE_OFLAG_OPEN);
    if(fd >= 0){
        if(ioctl(fd, LCD_CTRL_INIT_DEV, RT_NULL) == 0)
            rt_kprintf("LCD mode config finish.\n");
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_finalize(void) {
	close(fd);
	fd = -1;
    return JAVACALL_OK;
}

javacall_result javacall_directui_get_screen(int* screen_width, int* screen_height) {
    *screen_width = 208;
    *screen_height = 80;
    return JAVACALL_FAIL;
}

void javacall_directui_clear(int rgb) {
	struct rt_lcd_set_param set_param;

	set_param.zone_info.col_start = 0;
	set_param.zone_info.col_end = DISPLAY_BUFFER_COLUMU;
	set_param.zone_info.row_start = 0;
	set_param.zone_info.row_end = DISPLAY_BUFFER_ROW;
	set_param.value = 0x0;

	if(ioctl(fd, LCD_CTRL_SET_SECTION, &set_param) == 0) {
            rt_kprintf("LCD display cls.\n");
	}
}

javacall_result javacall_directui_flush() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_flush_region(int xstart, int ystart, int xend, int yend) {
	return JAVACALL_OK;
}

javacall_result javacall_directui_textout(int font, int color, int x, int y,
        const javacall_utf16* text, int textLen, int delayed) {
	int i;
	struct rt_lcd_refresh_param refresh_param;
	
	refresh_param.zone_info.col_start = x;
	refresh_param.zone_info.row_start = y;
	refresh_param.zone_info.row_end = refresh_param.zone_info.row_start + 3;
	for (i = 0; i < textLen; i++) {
		int skip = 0;
		refresh_param.zone_info.col_end = refresh_param.zone_info.col_start + 16;
		
		switch (text[i]) {
		case 0x5f53:
			refresh_param.p_data = (rt_uint8_t *)BigGB_B5B1;
			break;
		case 0x524d:
			refresh_param.p_data = (rt_uint8_t *)BigGB_C7B0;
			break;
		case 0x6b63:
			refresh_param.p_data = (rt_uint8_t *)BigGB_D5FD;
			break;
		case 0x5411:
			refresh_param.p_data = (rt_uint8_t *)BigGB_CFF2;
			break;
		case 0x6709:
			refresh_param.p_data = (rt_uint8_t *)BigGB_D3D0;
			break;
		case 0x529f:
			refresh_param.p_data = (rt_uint8_t *)BigGB_B9A6;
			break;
		default:
			skip = 1;
		}
		if (!skip) {
			ioctl(fd, LCD_CTRL_REFRESH_SECTION, &refresh_param);
		}
		refresh_param.zone_info.col_start = refresh_param.zone_info.col_end;
	}
	javacall_printf("\n");
	
	return JAVACALL_OK;
}

javacall_result javacall_directui_text_getsize(int font, const javacall_utf16* text,
        int textLen, int* width, int* height) {
    return JAVACALL_FAIL;
}

javacall_result javacall_directui_image_getsize(javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int* width, int* height) {
    return JAVACALL_FAIL;
}

javacall_result javacall_directui_drawimage(int x, int y, javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int delayed) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_drawrawdata(int x, int y, javacall_uint8* image_data,
        int w, int h, int delayed) {

    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_init() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_finalize() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_get(javacall_keypress_code* key, javacall_keypress_type* type) {
	return JAVACALL_OK;
}

javacall_result javacall_directui_setBacklight(int on) {
	return JAVACALL_OK;
}
#ifdef __cplusplus
}
#endif
