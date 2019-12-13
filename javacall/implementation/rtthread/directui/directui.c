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
#include "DefineCharLib.h"

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
#define LCD_CTRL_UPDATE_SECTION			6
#define LCD_CTRL_FLUSH					7
#define LCD_CTRL_CLEAR_SECTION			8
#define LCD_CTRL_FLUSH_PARTIAL			9

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
        ioctl(fd, LCD_CTRL_INIT_DEV, RT_NULL);
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_finalize(void) {
	close(fd);
	fd = -1;
    return JAVACALL_OK;
}

javacall_result javacall_directui_get_screen(int* screen_width, int* screen_height) {
    *screen_width = DISPLAY_BUFFER_COLUMU;
    *screen_height = 80;
    return JAVACALL_OK;
}

void javacall_directui_clear(int rgb) {
	struct rt_lcd_set_param set_param;

	set_param.zone_info.col_start = 0;
	set_param.zone_info.col_end = DISPLAY_BUFFER_COLUMU;
	set_param.zone_info.row_start = 0;
	set_param.zone_info.row_end = DISPLAY_BUFFER_ROW;
	set_param.value = 0x0;

	if(ioctl(fd, LCD_CTRL_SET_SECTION, &set_param) == 0) {
            //rt_kprintf("LCD display cls.\n");
	}
}

javacall_result javacall_directui_flush() {
	ioctl(fd, LCD_CTRL_FLUSH, NULL);
    return JAVACALL_OK;
}

javacall_result javacall_directui_flush_region(int xstart, int ystart, int xend, int yend) {
	struct rt_lcd_zone_info zone_info;

	zone_info.col_start = xstart;
	zone_info.col_end = xend;
	zone_info.row_start = ystart;
	zone_info.row_end = yend;
		
	ioctl(fd, LCD_CTRL_FLUSH_PARTIAL, &zone_info);
	return JAVACALL_OK;
}

static int valid_font(int size) {
	if (size < 2) {
		return 2;
	} else if (size > 4){
		return 4;
	}
	return size;
}

typedef struct {
	javacall_utf16 unicode;
	rt_uint8_t* data;
} FontInfo;

#define UNI_GB_B6C1    (u_int)0x8bfb
#define UNI_GB_BFA8    (u_int)0x5361
#define UNI_GB_D6D0    (u_int)0x4e2d
#define UNI_GB_B3C9    (u_int)0x6210
#define UNI_GB_B9A6    (u_int)0x529f
#define UNI_GB_CAA7    (u_int)0x5931
#define UNI_GB_B0DC    (u_int)0x8d25
#define UNI_GB_C0AD    (u_int)0x62c9
#define UNI_GB_D5A2    (u_int)0x95f8
#define UNI_GB_C7EB    (u_int)0x8bf7
#define UNI_GB_B9BA    (u_int)0x8d2d
#define UNI_GB_B5E7    (u_int)0x7535
#define UNI_GB_CDB8    (u_int)0x900f
#define UNI_GB_D6A7    (u_int)0x652f
#define UNI_GB_B6DA    (u_int)0x56e4
#define UNI_GB_BBFD    (u_int)0x79ef
#define UNI_GB_C4E6    (u_int)0x9006
#define UNI_GB_CFE0    (u_int)0x76f8
#define UNI_GB_D0F2    (u_int)0x5e8f
#define UNI_GB_C9CF    (u_int)0x4e0a
#define UNI_GB_D2BB    (u_int)0x4e00
#define UNI_GB_B6FE    (u_int)0x4e8c
#define UNI_GB_C8FD    (u_int)0x4e09
#define UNI_GB_CBC4    (u_int)0x56db
#define UNI_GB_CEE5    (u_int)0x4e94
#define UNI_GB_C1F9    (u_int)0x516d
#define UNI_GB_C6DF    (u_int)0x4e03
#define UNI_GB_B0CB    (u_int)0x516b
#define UNI_GB_BEC5    (u_int)0x4e5d
#define UNI_GB_CAAE    (u_int)0x5341
#define UNI_GB_D4C2    (u_int)0x6708
#define UNI_GB_D5FD    (u_int)0x6b63
#define UNI_GB_B7B4    (u_int)0x53cd
#define UNI_GB_CFF2    (u_int)0x5411
#define UNI_GB_D3D0    (u_int)0x6709
#define UNI_GB_CEDE    (u_int)0x65e0
#define UNI_GB_C1BF    (u_int)0x91cf
#define UNI_GB_D1B9    (u_int)0x538b
#define UNI_GB_C1F7    (u_int)0x6d41
#define UNI_GB_D7DC    (u_int)0x603b
#define UNI_GB_B5B1    (u_int)0x5f53
#define UNI_GB_C7B0    (u_int)0x524d
#define UNI_GB_C8D5    (u_int)0x65e5
#define UNI_GB_C6DA    (u_int)0x671f
#define UNI_GB_CAB1    (u_int)0x65f6
#define UNI_GB_BCE4    (u_int)0x95f4
#define UNI_GB_CAA3    (u_int)0x5269
#define UNI_GB_D3E0    (u_int)0x4f59
#define UNI_GB_BDF0    (u_int)0x91d1
#define UNI_GB_B6EE    (u_int)0x989d
#define UNI_GB_B1ED    (u_int)0x8868
#define UNI_GB_BAC5    (u_int)0x53f7
#define UNI_GB_B5D8    (u_int)0x5730
#define UNI_GB_D6B7    (u_int)0x5740
#define UNI_GB_C2CA    (u_int)0x7387
#define UNI_GB_D2F2    (u_int)0x56E0
#define UNI_GB_CAFD    (u_int)0x6570
#define UNI_GB_C4DC    (u_int)0x80fd

#define BIG_CHINESE_FONT_ENTRY(x) {UNI_GB_##x, BigGB_##x}

static FontInfo BigChineseFontTable[] = {
	/*��*/BIG_CHINESE_FONT_ENTRY(C9CF),
	/*һ*/BIG_CHINESE_FONT_ENTRY(D2BB),
	/*��*/BIG_CHINESE_FONT_ENTRY(B6FE),
	/*��*/BIG_CHINESE_FONT_ENTRY(C8FD),
	/*��*/BIG_CHINESE_FONT_ENTRY(CBC4),
	/*��*/BIG_CHINESE_FONT_ENTRY(CEE5),
	/*��*/BIG_CHINESE_FONT_ENTRY(C1F9),
	/*��*/BIG_CHINESE_FONT_ENTRY(C6DF),
	/*��*/BIG_CHINESE_FONT_ENTRY(B0CB),
	/*��*/BIG_CHINESE_FONT_ENTRY(BEC5),
	/*ʮ*/BIG_CHINESE_FONT_ENTRY(CAAE),
	/*��*/BIG_CHINESE_FONT_ENTRY(D4C2),
	/*��*/BIG_CHINESE_FONT_ENTRY(D5FD),
	/*��*/BIG_CHINESE_FONT_ENTRY(B7B4),
	/*��*/BIG_CHINESE_FONT_ENTRY(CFF2),
	/*��*/BIG_CHINESE_FONT_ENTRY(D3D0),
	/*��*/BIG_CHINESE_FONT_ENTRY(CEDE),	
	/*��*/BIG_CHINESE_FONT_ENTRY(B9A6),
	/*��*/BIG_CHINESE_FONT_ENTRY(B5E7),	
	/*��*/BIG_CHINESE_FONT_ENTRY(C1BF),
	/*ѹ*/BIG_CHINESE_FONT_ENTRY(D1B9),
	/*��*/BIG_CHINESE_FONT_ENTRY(C1F7),
	/*��*/BIG_CHINESE_FONT_ENTRY(D7DC),
	/*��*/BIG_CHINESE_FONT_ENTRY(B5B1),
	/*ǰ*/BIG_CHINESE_FONT_ENTRY(C7B0),
	/*��*/BIG_CHINESE_FONT_ENTRY(C8D5),
	/*��*/BIG_CHINESE_FONT_ENTRY(C6DA),
	/*ʱ*/BIG_CHINESE_FONT_ENTRY(CAB1),
	/*��*/BIG_CHINESE_FONT_ENTRY(BCE4),	
	/*͸*/BIG_CHINESE_FONT_ENTRY(CDB8),
	/*֧*/BIG_CHINESE_FONT_ENTRY(D6A7),
	/*ʣ*/BIG_CHINESE_FONT_ENTRY(CAA3),
	/*��*/BIG_CHINESE_FONT_ENTRY(D3E0),
	/*��*/BIG_CHINESE_FONT_ENTRY(BDF0),
	/*��*/BIG_CHINESE_FONT_ENTRY(B6EE),
	/*��*/BIG_CHINESE_FONT_ENTRY(B1ED),
	/*��*/BIG_CHINESE_FONT_ENTRY(BAC5),
	/*��*/BIG_CHINESE_FONT_ENTRY(B5D8),
	/*ַ*/BIG_CHINESE_FONT_ENTRY(D6B7),	
	/*��*/BIG_CHINESE_FONT_ENTRY(CFE0),
	/*��*/BIG_CHINESE_FONT_ENTRY(C2CA),
	/*��*/BIG_CHINESE_FONT_ENTRY(D2F2),
	/*��*/BIG_CHINESE_FONT_ENTRY(CAFD),
	/*��*/BIG_CHINESE_FONT_ENTRY(C4DC)
};

#define SMALL_CHINESE_FONT_ENTRY(x) {UNI_GB_##x, SmallGB_##x}

static FontInfo SmallChineseFontTable[] = {
	
	/*��*/SMALL_CHINESE_FONT_ENTRY(B6C1),
	/*��*/SMALL_CHINESE_FONT_ENTRY(BFA8),
	/*��*/SMALL_CHINESE_FONT_ENTRY(D6D0),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B3C9),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B9A6),
	/*ʧ*/SMALL_CHINESE_FONT_ENTRY(CAA7),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B0DC),
	/*��*/SMALL_CHINESE_FONT_ENTRY(C0AD),
	/*բ*/SMALL_CHINESE_FONT_ENTRY(D5A2),
	/*��*/SMALL_CHINESE_FONT_ENTRY(C7EB),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B9BA),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B5E7),
	/*͸*/SMALL_CHINESE_FONT_ENTRY(CDB8),
	/*֧*/SMALL_CHINESE_FONT_ENTRY(D6A7),
	/*��*/SMALL_CHINESE_FONT_ENTRY(B6DA),
	/*��*/SMALL_CHINESE_FONT_ENTRY(BBFD),
	/*��*/SMALL_CHINESE_FONT_ENTRY(C4E6),
	/*��*/SMALL_CHINESE_FONT_ENTRY(CFE0),
	/*��*/SMALL_CHINESE_FONT_ENTRY(D0F2)
	
};

static FontInfo UnitFontsTable[] = {
	{UYuan, UYuanPicture},
	{UWYuan, UWYuanPicture},
	{UdegreeA, UdegreeAPicture},
	{UdegreeC, UdegreeCPicture},
	{UW, UWPicture},
	{UkW, UkWPicture},
	{UVA, UVAPicture},
	{UkVA, UkVAPicture},
	{Uvar, UvarPicture},
	{Ukvar, UkvarPicture},
	{UkWh, UkWhPicture},
	{UkVAh, UkVAhPicture},
	{Ukvarh, UkvarhPicture},
	{UA, UAPicture},
	{UV, UVPicture},
	{UHz, UHzPicture},
	{U1PWh, U1PWhPicture},
	{U1Pvarh, U1PvarhPicture},
	{U1PVAh, U1PVAhPicture},
	{UAh, UAhPicture}
};

static rt_uint8_t* PictureFonts[] = {
	LabFlagPicture,
	SetupKeyPicture,
	MiMaErrorPicture,
	DianChi1Picture,
	DianChi2Picture,
	FeilvDianJiaTPicture,
	JieTiDianJiaLPicture,
	UaPicture,
	UbPicture,
	UcPicture,
	_IaPicture,
	IaPicture,
	_IbPicture,
	IbPicture,
	_IcPicture,
	IcPicture,
	FaSongPicture,
	FaSong1Picture,
	FaSong2Picture,
	SignalGrade0Picture,
	SignalGrade1Picture,
	SignalGrade2Picture,
	SignalGrade3Picture,
	SignalGrade4Picture,
	ZaiBoFaSongPicture,
	ZaiBoFaSongUpPicture,
	ZaiBoFaSongDownPicture,
	ZaiBoFaSongBothPicture
};

static rt_uint8_t* SmallNumberFonts[] = {
	SmallNumber0,
	SmallNumber1,
	SmallNumber2,
	SmallNumber3,
	SmallNumber4,
	SmallNumber5,
	SmallNumber6,
	SmallNumber7,
	SmallNumber8,
	SmallNumber9,
	SmallNumberA,
	SmallNumberB,
	SmallNumberC,
	SmallNumberD,
	SmallNumberE,
	SmallNumberF,
};

static rt_uint8_t* BigNumberFonts[] = {
	BigNumber0,
	BigNumber1,
	BigNumber2,
	BigNumber3,
	BigNumber4,
	BigNumber5,
	BigNumber6,
	BigNumber7,
	BigNumber8,
	BigNumber9,
	BigNumberA,
	BigNumberB,
	BigNumberC,
	BigNumberD,
	BigNumberE,
	BigNumberF
};

static rt_uint8_t* get_Chinese_fonts(int font, javacall_utf16 unicode) {
	int table_len, i;
	FontInfo* table;
	
	if (font == 2) {
		table_len = sizeof(SmallChineseFontTable)/sizeof(FontInfo);
		table = SmallChineseFontTable;
	} else if (font == 3) {
		table_len = sizeof(BigChineseFontTable)/sizeof(FontInfo);
		table = BigChineseFontTable;
	} else {
		return NULL;
	}
	
	for (i = 0; i < table_len; i++) {
		if (table[i].unicode == unicode) {
			return table[i].data;
		}
	}
	
	return NULL;
}

static rt_uint8_t* get_font_data(int font, javacall_utf16 unicode) {
	if ((unicode >= LabFlag) && (unicode <= ZaiBoFaSongBoth)) {
		if (font == 2) {
			rt_kprintf("get_font_data: %d\n", unicode);
			return PictureFonts[unicode - LabFlag];
		} else {
			return NULL;
		}
	}
	
	if ((unicode >= 0x0200) && (unicode <= 0x02FF)) {
		if (font == 2) {
			int i;
			FontInfo* table = UnitFontsTable;
			int table_len = sizeof(UnitFontsTable)/sizeof(FontInfo);
			for (i = 0; i < table_len; i++) {
				if (table[i].unicode == unicode) {
					return table[i].data;
				}
			}
		}
		return NULL;
	}
	
	if ((unicode >= (javacall_utf16)'0') && ((unicode <= (javacall_utf16)'9'))) {
		if (font == 2) {
			return SmallNumberFonts[unicode - (javacall_utf16)'0'];
		} else if (font == 4) {
			return BigNumberFonts[unicode - (javacall_utf16)'0'];
		} else {
			return NULL;
		}
	}
	
	if ((unicode >= (javacall_utf16)'A') && ((unicode <= (javacall_utf16)'F'))) {
		if (font == 2) {
			return SmallNumberFonts[unicode - (javacall_utf16)'A' + 10];
		} else if (font == 4) {
			return BigNumberFonts[unicode - (javacall_utf16)'A' + 10];
		} else {
			return NULL;
		}
	}

	if (unicode == (javacall_utf16)'.') {
		if (font == 4) {
			return BigNumberSPoint;
		} else {
			return NULL;
		}
	}

	if (unicode == (javacall_utf16)':') {
		if (font == 4) {
			return BigNumberTPoint;
		} else {
			return NULL;
		}
	}

	if (unicode == (javacall_utf16)'-') {
		if (font == 4) {
			return BigNumberNegative;
		} else {
			return NULL;
		}
	}
	
	return get_Chinese_fonts(font, unicode);
}

static get_margin(int font) {
	return font == 4?2:0;
}

javacall_result javacall_directui_textout(int font, int color, int x, int y,
        const javacall_utf16* text, int textLen, int delayed) {
	int i;
	struct rt_lcd_refresh_param refresh_param;
	
	font = valid_font(font);
	
	refresh_param.zone_info.col_start = x;
	refresh_param.zone_info.row_start = y;
	refresh_param.zone_info.row_end = refresh_param.zone_info.row_start + font;
	for (i = 0; (i < textLen) && (refresh_param.zone_info.col_start < DISPLAY_BUFFER_COLUMU); i++) {
		int w, h;
		javacall_directui_text_getsize(font, text+i, 1, &w, &h);
		refresh_param.zone_info.col_end = refresh_param.zone_info.col_start + w - get_margin(font);
		rt_kprintf("javacall_directui_textout: 0x%x, font=%d, width=%d\n", text[i], font, w);
		
		refresh_param.p_data = get_font_data(font, text[i]);
		if (refresh_param.p_data != NULL) {
			ioctl(fd, LCD_CTRL_UPDATE_SECTION, &refresh_param);
		} else {
			struct rt_lcd_set_param set_param;

			memcpy(&set_param.zone_info, &refresh_param.zone_info, sizeof(struct rt_lcd_zone_info));
			set_param.value = 0x0;

			ioctl(fd, LCD_CTRL_CLEAR_SECTION, &set_param);
		}
		refresh_param.zone_info.col_start = refresh_param.zone_info.col_end + get_margin(font);
	}

	if (!delayed) {
		struct rt_lcd_zone_info zone_info;

		memcpy(&zone_info, &refresh_param.zone_info, sizeof(struct rt_lcd_zone_info));
		
		ioctl(fd, LCD_CTRL_FLUSH_PARTIAL, &zone_info);
	}
	
	return JAVACALL_OK;
}

static int get_char_width(int font, javacall_utf16 unicode) {
	int size;
	
	if ((unicode >= 0x0100) && (unicode <= 0x01FF)) {
		//PictureChar
		size = 12;
	} else if ((unicode >= 0x0200) && (unicode <= 0x02FF)) {
		//UnitChar
		size = 20;
	} else if (font == 2) {
		size = 12;
	} else if (font == 3) {
		size = 16;
	} else {
		if ((unicode == (javacall_utf16)'.') || (unicode == (javacall_utf16)':')) {
			size = 4;
		} else {
			size= 14;
		}
	}

	return size + get_margin(font);
}

javacall_result javacall_directui_text_getsize(int font, const javacall_utf16* text,
        int textLen, int* width, int* height) {
	
	int w = 0, i;
	
    font = valid_font(font);
	
	for (i = 0; i < textLen; i++) {
		w += get_char_width(font, text[i]);
	}
	
	*width = w;
	*height = font;
	
	return JAVACALL_OK;
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
