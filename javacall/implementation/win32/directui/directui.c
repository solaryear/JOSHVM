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
#include <windows.h>

#ifdef __cplusplus
extern "C"{
#endif

static HBITMAP hDefaultDisplayCanvas;
static HDC hDefaultDisplayHdc;
static HBITMAP hbmOld;
static HWND hDefaultDisplayWnd;
static int _down_key = -1;
static javacall_keypress_type _down_keyevent;
static int direct_ui_initialized = 0;

#define SCREEN_WIDTH 208
#define SCREEN_HEIGHT 80

/* convert color to 16bit color */
#define RGB16TORGB24(x) (((( x ) << 8) & 0x00F80000) | \
                         ((( x ) << 5) & 0x0000FC00) | \
                         ((( x ) << 3) & 0x000000F8) )

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
        BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hDefaultDisplayHdc, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		javacall_printf("CHAR %c\n", wParam);
		if ((wParam >= 0x30) && (wParam <= 0x39)) {
			_down_key = wParam;
		} else if (wParam == VK_F1) {
			_down_key = JAVACALL_KEY_FUNC1;
		} else if (wParam == VK_F2) {
			_down_key = JAVACALL_KEY_FUNC2;
		} else if (wParam == VK_F3) {
			_down_key = JAVACALL_KEY_FUNC3;
		} else if (wParam == VK_F4) {
			_down_key = JAVACALL_KEY_FUNC4;
		} else if (wParam == VK_RETURN) {
			_down_key = JAVACALL_KEY_OK;
		} else if (wParam == VK_OEM_PERIOD) {
			_down_key = JAVACALL_KEY_DOT;
		} else if (wParam == VK_BACK) {
			_down_key = JAVACALL_KEY_BACK;
		} else {
			break;
		}
		_down_keyevent = (message == WM_KEYDOWN)?JAVACALL_KEYPRESSED:JAVACALL_KEYRELEASED;
		javanotify_key_event(-1, JAVACALL_KEYPRESSED);

		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

static javacall_result init_window(void) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = "JOSHSCRCLS";
    wcex.hIconSm        = NULL;

    if (!RegisterClassEx(&wcex))
    {
        return JAVACALL_FAIL;
    }

    HWND hWnd = CreateWindow(
        "JOSHSCRCLS",
        "EMUSCR",
        WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT,
        SCREEN_WIDTH+16, SCREEN_HEIGHT+48,
        NULL,  
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
    	return JAVACALL_FAIL;
    }

	HDC hdc = GetDC(hWnd);
	HDC hdcMem = CreateCompatibleDC(hdc);
	hDefaultDisplayCanvas = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
    hbmOld = SelectObject(hdcMem, hDefaultDisplayCanvas);
	ReleaseDC(hWnd, hdc);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
	hDefaultDisplayWnd = hWnd;
	hDefaultDisplayHdc = hdcMem;
    return JAVACALL_OK;
}

static javacall_result ensure_initialized() {
	if (!direct_ui_initialized) {
		if (JAVACALL_OK == init_window()) {
			javacall_directui_flush();
			direct_ui_initialized = 1;
		}
	}

	return direct_ui_initialized?JAVACALL_OK:JAVACALL_FAIL;
}

javacall_result javacall_directui_init(void) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_finalize(void) {
	if (direct_ui_initialized) {
		SelectObject(hDefaultDisplayHdc, hbmOld);
		DeleteObject(hDefaultDisplayCanvas);
		DeleteDC(hDefaultDisplayHdc);
		DestroyWindow(hDefaultDisplayWnd);
		direct_ui_initialized = 0;
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_get_screen(int* screen_width, int* screen_height) {
	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}
    *screen_width = SCREEN_WIDTH;
    *screen_height = SCREEN_HEIGHT;
    return JAVACALL_OK;
}

void javacall_directui_clear(int rgb) {
	if (JAVACALL_OK != ensure_initialized()) {
		return;
	}
	HBRUSH hBrush = CreateSolidBrush(RGB((rgb>>16)&0xff,(rgb>>8)&0xff,rgb&0xff));
	RECT scrrect = {0,0,SCREEN_WIDTH, SCREEN_HEIGHT};
	FillRect(hDefaultDisplayHdc, &scrrect, hBrush);
	DeleteObject(hBrush);
}

javacall_result javacall_directui_flush() {
	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}
	InvalidateRect(hDefaultDisplayWnd, NULL, FALSE);
    return JAVACALL_OK;
}

javacall_result javacall_directui_flush_region(int xstart, int ystart, int xend, int yend) {
	RECT scrrect;
	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}

	scrrect.left = xstart;
	scrrect.top = ystart;
	scrrect.right = xend-1;
	scrrect.left = yend-1;

	InvalidateRect(hDefaultDisplayWnd, &scrrect, FALSE);
	return JAVACALL_OK;
}
#if 0
static int font_h(int size) {
	if (size < 2) {
		return 2*8;
	} else if (size > 4){
		return 4*8;
	}
	return size*8;
}

static int font_w(int size) {
	if (size <= 2) {
		return 8;
	} else if (size >= 4){
		return 10;
	}
	return 12;
}

javacall_result javacall_directui_textout(int font, int color, int x, int y,
        const javacall_utf16* text, int textLen, int delayed) {
    if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}
	HFONT hFont, hOldFont;

	hFont = CreateFont(font_h(font),font_w(font),0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
	                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,NULL);


	// Select the variable stock font into the specified device context.
	if (hOldFont = (HFONT)SelectObject(hDefaultDisplayHdc, hFont))
	{
	    TextOutW(hDefaultDisplayHdc, x,y*8, text, textLen);

	    // Restore the original font.
	    SelectObject(hDefaultDisplayHdc, hOldFont);
	}
	DeleteObject(hFont);

	wprintf(L"javacall_directui_textout: %s, x=%d, y=%d, textLen=%d, delayed=%d\n", text, x, y, textLen, delayed);
	//if (!delayed) {
		javacall_directui_flush();
	//}
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

static get_margin(int font) {
	return font == 4?2:0;
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
#endif

void drawPixel(int x, int y, int color) {
	COLORREF c;
	if (color) {
		c = RGB(0,0,0);
	} else {
		c = RGB(255,255,255);
	}
	SetPixel(hDefaultDisplayHdc, x, y, c);
}

javacall_result javacall_directui_image_getsize(javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int* width, int* height) {
    if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}

	if (type == JAVACALL_IMAGETYPE_JPG) {
		void *info = JPEG_To_RGB_init();
		if (info) {
    		if (JPEG_To_RGB_decodeHeader(info, image_data, data_len, width, height)) {
				return JAVACALL_OK;
    		} else {
				return JAVACALL_FAIL;
			}
			JPEG_To_RGB_free(info);
		} else {
			return JAVACALL_FAIL;
		}
    } else {
		return JAVACALL_NOT_IMPLEMENTED;
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_drawimage(int x, int y, javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int delayed) {
    int w, h;
	javacall_uint8 *raw_image;

	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}

    if (type == JAVACALL_IMAGETYPE_JPG) {
		void *info = JPEG_To_RGB_init();
		if (info) {
    		raw_image = JPEG_To_RGB_decode(info, image_data, data_len, &w, &h);
			if (raw_image != NULL) {
				unsigned char *p = raw_image;
				for (int j = 0; j < h; j++) {
					for (int i = 0; i < w; i++) {
						unsigned int color = *(unsigned int*)p;
						color = color & 0xffffff;
						p += 3; //pixel size is set to 3 by jpeblib

						SetPixel(hDefaultDisplayHdc, x+i, y+j, color);
					}
				}
				if (!delayed) {
					javacall_directui_flush();
				}
				free(raw_image);
			}
			JPEG_To_RGB_free(info);
		} else {
			return JAVACALL_FAIL;
		}
    } else {
		return JAVACALL_NOT_IMPLEMENTED;
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_drawrawdata(int x, int y, javacall_uint8* image_data,
        int w, int h, int delayed) {

	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}

    unsigned short *p = (unsigned short*)image_data;
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			SetPixel(hDefaultDisplayHdc, x+i, y+j, RGB16TORGB24(*p));
			p++;
		}
	}
	if (!delayed) {
		javacall_directui_flush();
	}
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_init() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_finalize() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_get(javacall_keypress_code* key, javacall_keypress_type* type) {
	if (JAVACALL_OK != ensure_initialized()) {
		return JAVACALL_FAIL;
	}

	printf("javacall_directui_key_event_get: _down_key=%d, _down_keyevent=%d\n", _down_key, _down_keyevent);

	*type = _down_keyevent;

	if (_down_key == -1) {
		return JAVACALL_WOULD_BLOCK;
	} else {
		*key = _down_key;
		_down_key = -1;
		return JAVACALL_OK;
	}
}

javacall_result javacall_directui_setBacklight(int on) {
	return JAVACALL_OK;
}


#ifdef __cplusplus
}
#endif
