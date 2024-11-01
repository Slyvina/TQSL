// License:
// 	TQSL/Source/TQSE.cpp
// 	Slyvina - Tricky's Quick SDL2 Event handler
// 	version: 24.10.28
// 
// 	Copyright (C) 2022, 2023, 2024 Jeroen P. Broks
// 
// 	This software is provided 'as-is', without any express or implied
// 	warranty.  In no event will the authors be held liable for any damages
// 	arising from the use of this software.
// 
// 	Permission is granted to anyone to use this software for any purpose,
// 	including commercial applications, and to alter it and redistribute it
// 	freely, subject to the following restrictions:
// 
// 	1. The origin of this software must not be misrepresented; you must not
// 	   claim that you wrote the original software. If you use this software
// 	   in a product, an acknowledgment in the product documentation would be
// 	   appreciated but is not required.
// 	2. Altered source versions must be plainly marked as such, and must not be
// 	   misrepresented as being the original software.
// 	3. This notice may not be removed or altered from any source distribution.
// End License

#include <TQSE.hpp>
#include <SlyvString.hpp>

namespace Slyvina {
	namespace TQSE {

		static std::string TQSE_AppTitle = "Slyvina - TQSE Application";

		static const int maxmousebuttons = 16;

		static bool TQSE_InitDone = false;

		static bool stAppTerminate = false;

		static std::map<int, bool> stKeyDown;
		static std::map<int, bool> stKeyHit;
		static std::map<int, bool> stKeyOldDown;		

		static bool MsButDown[maxmousebuttons];
		static bool MsButOldDown[maxmousebuttons];
		static bool MsButHit[maxmousebuttons];

		static bool MsMousePressed[maxmousebuttons];
		static bool MsMouseReleased[maxmousebuttons];

		static SDL_MouseWheelEvent wheel;
		static bool wheelused{ false };

		

		// keys
		static const int numkeys = 240;
		static const int stAllKeys[numkeys]{
			SDLK_UNKNOWN, // That is 0,
			SDLK_RETURN, // That is '\r',
			SDLK_ESCAPE, // That is '\033',
			SDLK_BACKSPACE, // That is '\b',
			SDLK_TAB, // That is '\t',
			SDLK_SPACE, // That is ' ',
			SDLK_EXCLAIM, // That is '!',
			SDLK_QUOTEDBL, // That is '"',
			SDLK_HASH, // That is '#',
			SDLK_PERCENT, // That is '%',
			SDLK_DOLLAR, // That is '$',
			SDLK_AMPERSAND, // That is '&',
			SDLK_QUOTE, // That is '\'',
			SDLK_LEFTPAREN, // That is '(',
			SDLK_RIGHTPAREN, // That is ')',
			SDLK_ASTERISK, // That is '*',
			SDLK_PLUS, // That is '+',
			SDLK_COMMA, // That is ',',
			SDLK_MINUS, // That is '-',
			SDLK_PERIOD, // That is '.',
			SDLK_SLASH, // That is '/',
			SDLK_0, // That is '0',
			SDLK_1, // That is '1',
			SDLK_2, // That is '2',
			SDLK_3, // That is '3',
			SDLK_4, // That is '4',
			SDLK_5, // That is '5',
			SDLK_6, // That is '6',
			SDLK_7, // That is '7',
			SDLK_8, // That is '8',
			SDLK_9, // That is '9',
			SDLK_COLON, // That is ':',
			SDLK_SEMICOLON, // That is ';',
			SDLK_LESS, // That is '<',
			SDLK_EQUALS, // That is ', // That is',
			SDLK_GREATER, // That is '>',
			SDLK_QUESTION, // That is '?',
			SDLK_AT, // That is '@',
			/*
			   Skip uppercase letters
			 */
			SDLK_LEFTBRACKET, // That is '[',
			SDLK_BACKSLASH, // That is '\\',
			SDLK_RIGHTBRACKET, // That is ']',
			SDLK_CARET, // That is '^',
			SDLK_UNDERSCORE, // That is '_',
			SDLK_BACKQUOTE, // That is '`',
			SDLK_a, // That is 'a',
			SDLK_b, // That is 'b',
			SDLK_c, // That is 'c',
			SDLK_d, // That is 'd',
			SDLK_e, // That is 'e',
			SDLK_f, // That is 'f',
			SDLK_g, // That is 'g',
			SDLK_h, // That is 'h',
			SDLK_i, // That is 'i',
			SDLK_j, // That is 'j',
			SDLK_k, // That is 'k',
			SDLK_l, // That is 'l',
			SDLK_m, // That is 'm',
			SDLK_n, // That is 'n',
			SDLK_o, // That is 'o',
			SDLK_p, // That is 'p',
			SDLK_q, // That is 'q',
			SDLK_r, // That is 'r',
			SDLK_s, // That is 's',
			SDLK_t, // That is 't',
			SDLK_u, // That is 'u',
			SDLK_v, // That is 'v',
			SDLK_w, // That is 'w',
			SDLK_x, // That is 'x',
			SDLK_y, // That is 'y',
			SDLK_z, // That is 'z',
			SDLK_CAPSLOCK, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK),
			SDLK_F1, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1),
			SDLK_F2, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2),
			SDLK_F3, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3),
			SDLK_F4, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4),
			SDLK_F5, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5),
			SDLK_F6, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6),
			SDLK_F7, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7),
			SDLK_F8, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8),
			SDLK_F9, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9),
			SDLK_F10, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10),
			SDLK_F11, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11),
			SDLK_F12, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12),
			SDLK_PRINTSCREEN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN),
			SDLK_SCROLLLOCK, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK),
			SDLK_PAUSE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE),
			SDLK_INSERT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT),
			SDLK_HOME, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME),
			SDLK_PAGEUP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP),
			SDLK_DELETE, // That is '\177',
			SDLK_END, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END),
			SDLK_PAGEDOWN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN),
			SDLK_RIGHT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT),
			SDLK_LEFT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT),
			SDLK_DOWN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN),
			SDLK_UP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP),
			SDLK_NUMLOCKCLEAR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR),
			SDLK_KP_DIVIDE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE),
			SDLK_KP_MULTIPLY, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY),
			SDLK_KP_MINUS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS),
			SDLK_KP_PLUS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS),
			SDLK_KP_ENTER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER),
			SDLK_KP_1, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1),
			SDLK_KP_2, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2),
			SDLK_KP_3, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3),
			SDLK_KP_4, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4),
			SDLK_KP_5, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5),
			SDLK_KP_6, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6),
			SDLK_KP_7, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7),
			SDLK_KP_8, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8),
			SDLK_KP_9, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9),
			SDLK_KP_0, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0),
			SDLK_KP_PERIOD, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD),
			SDLK_APPLICATION, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION),
			SDLK_POWER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER),
			SDLK_KP_EQUALS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS),
			SDLK_F13, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13),
			SDLK_F14, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14),
			SDLK_F15, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15),
			SDLK_F16, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16),
			SDLK_F17, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17),
			SDLK_F18, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18),
			SDLK_F19, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19),
			SDLK_F20, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20),
			SDLK_F21, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21),
			SDLK_F22, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22),
			SDLK_F23, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23),
			SDLK_F24, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24),
			SDLK_EXECUTE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE),
			SDLK_HELP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP),
			SDLK_MENU, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU),
			SDLK_SELECT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT),
			SDLK_STOP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP),
			SDLK_AGAIN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN),
			SDLK_UNDO, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO),
			SDLK_CUT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT),
			SDLK_COPY, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY),
			SDLK_PASTE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE),
			SDLK_FIND, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND),
			SDLK_MUTE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE),
			SDLK_VOLUMEUP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP),
			SDLK_VOLUMEDOWN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN),
			SDLK_KP_COMMA, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA),
			SDLK_KP_EQUALSAS400, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400),
			SDLK_ALTERASE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE),
			SDLK_SYSREQ, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ),
			SDLK_CANCEL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL),
			SDLK_CLEAR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR),
			SDLK_PRIOR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR),
			SDLK_RETURN2, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2),
			SDLK_SEPARATOR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR),
			SDLK_OUT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT),
			SDLK_OPER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER),
			SDLK_CLEARAGAIN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN),
			SDLK_CRSEL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL),
			SDLK_EXSEL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL),
			SDLK_KP_00, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00),
			SDLK_KP_000, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000),
			SDLK_THOUSANDSSEPARATOR, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR),        
			SDLK_DECIMALSEPARATOR, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR),
			SDLK_CURRENCYUNIT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT),
			SDLK_CURRENCYSUBUNIT, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT),
			SDLK_KP_LEFTPAREN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN),
			SDLK_KP_RIGHTPAREN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN),
			SDLK_KP_LEFTBRACE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE),
			SDLK_KP_RIGHTBRACE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE),
			SDLK_KP_TAB, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB),
			SDLK_KP_BACKSPACE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE),
			SDLK_KP_A, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A),
			SDLK_KP_B, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B),
			SDLK_KP_C, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C),
			SDLK_KP_D, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D),
			SDLK_KP_E, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E),
			SDLK_KP_F, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F),
			SDLK_KP_XOR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR),
			SDLK_KP_POWER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER),
			SDLK_KP_PERCENT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT),
			SDLK_KP_LESS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS),
			SDLK_KP_GREATER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER),
			SDLK_KP_AMPERSAND, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND),
			SDLK_KP_DBLAMPERSAND, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND),
			SDLK_KP_VERTICALBAR, // That is        SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR),
			SDLK_KP_DBLVERTICALBAR, // That is    SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR),
			SDLK_KP_COLON, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON),
			SDLK_KP_HASH, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH),
			SDLK_KP_SPACE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE),
			SDLK_KP_AT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT),
			SDLK_KP_EXCLAM, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM),
			SDLK_KP_MEMSTORE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE),
			SDLK_KP_MEMRECALL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL),
			SDLK_KP_MEMCLEAR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR),
			SDLK_KP_MEMADD, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD),
			SDLK_KP_MEMSUBTRACT, // That is   SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT),
			SDLK_KP_MEMMULTIPLY, // That is       SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY),
			SDLK_KP_MEMDIVIDE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE),
			SDLK_KP_PLUSMINUS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS),
			SDLK_KP_CLEAR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR),
			SDLK_KP_CLEARENTRY, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY),
			SDLK_KP_BINARY, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY),
			SDLK_KP_OCTAL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL),
			SDLK_KP_DECIMAL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL),
			SDLK_KP_HEXADECIMAL, // That is  SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL),

			SDLK_LCTRL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL),
			SDLK_LSHIFT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT),
			SDLK_LALT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT),
			SDLK_LGUI, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI),
			SDLK_RCTRL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL),
			SDLK_RSHIFT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT),
			SDLK_RALT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT),
			SDLK_RGUI, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI),

			SDLK_MODE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE),

			SDLK_AUDIONEXT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT),
			SDLK_AUDIOPREV, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV),
			SDLK_AUDIOSTOP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP),
			SDLK_AUDIOPLAY, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY),
			SDLK_AUDIOMUTE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE),
			SDLK_MEDIASELECT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT),
			SDLK_WWW, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW),
			SDLK_MAIL, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL),
			SDLK_CALCULATOR, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR),
			SDLK_COMPUTER, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER),
			SDLK_AC_SEARCH, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH),
			SDLK_AC_HOME, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME),
			SDLK_AC_BACK, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK),
			SDLK_AC_FORWARD, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD),
			SDLK_AC_STOP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP),
			SDLK_AC_REFRESH, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH),
			SDLK_AC_BOOKMARKS, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS),

			SDLK_BRIGHTNESSDOWN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN),
			SDLK_BRIGHTNESSUP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP),
			SDLK_DISPLAYSWITCH, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH),
			SDLK_KBDILLUMTOGGLE, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE),
			SDLK_KBDILLUMDOWN, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN),
			SDLK_KBDILLUMUP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP),
			SDLK_EJECT, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT),
			SDLK_SLEEP, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP),
			SDLK_APP1, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP1),
			SDLK_APP2, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP2),
			SDLK_AUDIOREWIND, // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOREWIND),
			SDLK_AUDIOFASTFORWARD // That is SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOFASTFORWARD)                
		};


		std::string AllKeys() {
			std::string ret{ "Keycodes\n" };
			for (int i = 0; i < numkeys; ++i)
				ret += Units::TrSPrintF("%03d> %09d", i, stAllKeys[i]);
			return ret;
		}


		static void MouseClean(bool full = false) {
			for (unsigned int i = 0; i < maxmousebuttons; i++) {
				if (full) MsButOldDown[i] = false; else MsButOldDown[i] = MsButDown[i];
				if (full) MsButDown[i] = false;
				MsButHit[i] = false;
				MsMousePressed[i] = false;
				MsMouseReleased[i] = false;				
			}
		}

		static void KeyClean(bool full = false) {
			//for (auto& i : stAllKeys) {
			for (unsigned int j = 0; j < numkeys; j++) {
				auto i = stAllKeys[j];
				if (full) {
					stKeyDown[i] = false;
					stKeyOldDown[i] = false;
				} else stKeyOldDown[i] = stKeyDown[i];
				stKeyHit[i] = false;
			}
		}

		void Poll(EventFunction EventCallBack) {
			// All initiated?
			if (!TQSE_InitDone) {
				KeyClean(true);
				MouseClean(true);				
				TQSE_InitDone = true;
			}
			// Let's get ready to rumble!
			KeyClean();
			MouseClean();
			stAppTerminate = false;
			wheelused = false;
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0) {
				switch (e.type) {
				case SDL_KEYDOWN: {
					auto pkey = e.key.keysym.sym;
					//std::cout << "KeyDown:" << (int)pkey << "\n"; // debug only
					stKeyDown[pkey] = true;
					stKeyHit[pkey] = stKeyDown[pkey] && (!stKeyOldDown[pkey]);
					//printf("DOWN: %d\n",pkey);
					break;
				}
				case SDL_KEYUP: {
					auto pkey = e.key.keysym.sym;
					//printf("UP:   %d\n", pkey);
					stKeyDown[pkey] = false;
					break;
				}
				case SDL_MOUSEBUTTONDOWN: {
					auto pbut = e.button.button;
					MsButDown[pbut] = true;
					MsButHit[pbut] = MsButDown[pbut] && (!MsButOldDown[pbut]);
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					auto pbut = e.button.button;
					MsButDown[pbut] = false;
					MsMouseReleased[pbut] = true;
					break;
				}
				case SDL_MOUSEWHEEL: {
					wheel = e.wheel;
					wheelused = true;
					break;
				}

				case SDL_QUIT:
					stAppTerminate = true;
					break;
				}
				if (EventCallBack) EventCallBack(&e);
			}
		}

		bool AppTerminate() { return stAppTerminate; }

		bool KeyHit(SDL_KeyCode c) { return stKeyHit[c]; }

		bool KeyDown(SDL_KeyCode c) { return stKeyDown[c]; }

		int GetMouseButtons() { return maxmousebuttons; }

		SDL_KeyCode GetKey() {
			for (auto& c : stKeyHit) if (c.second) return (SDL_KeyCode)c.first;
			return SDLK_UNKNOWN;
		}

		unsigned char GetChar() {
			auto shift{ KeyDown(SDLK_LSHIFT) || KeyDown(SDLK_RSHIFT) };
			auto key = GetKey();
			switch (key) {
			case SDLK_TAB:
			case SDLK_BACKSPACE:
			case SDLK_SPACE:
			case SDLK_RETURN:
				return (unsigned char)key;
			case SDLK_0:
				if (shift) return ')'; else return '0';
			case SDLK_1:
				if (shift) return '!'; else return '1';
			case SDLK_2:
				if (shift) return '@'; else return '2';
			case SDLK_3:
				if (shift) return '#'; else return '3';
			case SDLK_4:
				if (shift) return '$'; else return '4';
			case SDLK_5:
				if (shift) return '%'; else return '5';
			case SDLK_6:
				if (shift) return '^'; else return '6';
			case SDLK_7:
				if (shift) return '&'; else return '7';
			case SDLK_8:
				if (shift) return '*'; else return '8';
			case SDLK_9:
				if (shift) return '('; else return '9';
			case SDLK_BACKQUOTE:
				if (shift) return '`'; else return '~';
			case SDLK_UNDERSCORE:
			case SDLK_MINUS:
				if (shift) return '_'; else return '-';
			case SDLK_PLUS:
			case SDLK_EQUALS:
				if (shift) return '+'; else return '=';
			case SDLK_BACKSLASH:
				//if (shift) return '|'; else return '\\';
				return '\\'; // The | character has a special meaning and should therefore not be used.
			case SDLK_LEFTBRACKET:
				if (shift) return '{'; else return '[';
			case SDLK_RIGHTBRACKET:
				if (shift) return '}'; else return ']';
			case SDLK_COLON:
			case SDLK_SEMICOLON:
				if (shift) return ':'; else return ';';
			case SDLK_QUOTE:
				if (shift) return '"'; else return '\'';
			case SDLK_COMMA:
				if (shift) return '<'; else return ',';
			case SDLK_PERIOD:
				if (shift) return '>'; else return '.';
			case SDLK_SLASH:
				if (shift) return '?'; else return '/';
			case SDLK_KP_0: return '0';
			case SDLK_KP_1: return '1';
			case SDLK_KP_2: return '2';
			case SDLK_KP_3: return '3';
			case SDLK_KP_4: return '4';
			case SDLK_KP_5: return '5';
			case SDLK_KP_6: return '6';
			case SDLK_KP_7: return '7';
			case SDLK_KP_8: return '8';
			case SDLK_KP_9: return '9';
			case SDLK_KP_ENTER: return '\r';
			case SDLK_KP_MINUS: return '-';
			case SDLK_KP_MULTIPLY: return '*';
			case SDLK_KP_DIVIDE: return '/';
			case SDLK_KP_PLUS: return '+';
			case SDLK_KP_DECIMAL: return '.';

			default:
				if (key >= SDLK_a && key <= SDLK_z) {
					if (shift) return (unsigned char)(key - 32);
					return (unsigned char)key;
				}
			}
			return 0;
		}



		int MouseX() {
			int x, y;
			SDL_GetMouseState(&x, &y);
			return x;
		}

		int MouseY() {
			int x, y;
			SDL_GetMouseState(&x, &y);
			return y;
		}

		void HideMouse() {
			SDL_ShowCursor(SDL_DISABLE);
		}

		void ShowMouse() {
			SDL_ShowCursor(SDL_ENABLE);
		}

		bool MouseDown(int code) {
			if (code < 0 || code >= maxmousebuttons) return false;
			return MsButDown[code];
		}

		bool MouseHit(int code) {
			if (code < 0 || code >= maxmousebuttons) return false;
			return MsButHit[code];
		}

		bool MouseReleased(int c) {
			return MsMouseReleased[(SDL_KeyCode)c];
		}

		int MouseWheelY() {
			if (!wheelused)
				return 0;
			else
				return wheel.y;
		}


		int KeyByName(std::string name) {
			return SDL_GetKeyFromName(name.c_str());
		}

		void ShowKeyNames() {
			for (auto kc : stAllKeys) {
				printf("%10s = %9d\n", SDL_GetKeyName(kc), kc);
			}
		}

		bool Yes(std::string question) {
			const SDL_MessageBoxButtonData buttons[] = {
				//{ /* .flags, .buttonid, .text */        0, 0, "no" },
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
			};
			const SDL_MessageBoxColorScheme colorScheme = {
				{ /* .colors (.r, .g, .b) */
					/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
					{ 100,   0,   0 },
					/* [SDL_MESSAGEBOX_COLOR_TEXT] */
					{  255, 180,   0 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
					{ 255, 255,   0 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
					{   0,   0, 255 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
					{ 255,   0, 255 }
				}
			};
			const SDL_MessageBoxData messageboxdata = {
				SDL_MESSAGEBOX_INFORMATION, /* .flags */
				NULL, /* .window */
				"Notice", /* .title */
				question.c_str(), /* .message */
				SDL_arraysize(buttons), /* .numbuttons */
				buttons, /* .buttons */
				&colorScheme /* .colorScheme */
			};
			int buttonid{ 0 };
			if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
				//SDL_Log("error displaying message box");
				std::cout << "SDL Message Box Failure\n";
				return false;

			}
			if (buttonid == -1) {
				//SDL_Log("no selection");
			} else {
				//SDL_Log("selection was %s", buttons[buttonid].text);
				//std::cout << "User selected " << buttonid << " -> " << buttons[buttonid].text << std::endl;
			}
			switch (buttonid) {
			case -1:
			case 1:
				return false;
			case 0:
				return true;
			default:
				std::cout << "Is the input right? I don't think so!\x7\n";
				return false;
				break;
			}
		}

		void Notify(std::string message) {
			const SDL_MessageBoxButtonData buttons[] = {
				//{ /* .flags, .buttonid, .text */        0, 0, "no" },
				//{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes" },
				//{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "cancel" },
					{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,1,"Ok"}
			};
			const SDL_MessageBoxColorScheme colorScheme = {
				{ /* .colors (.r, .g, .b) */
					/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
					{ 0,   0,   0 },
					/* [SDL_MESSAGEBOX_COLOR_TEXT] */
					{   255, 180,   0 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
					{ 255, 255,   0 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
					{   0,   0, 0 },
					/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
					{ 255,   255, 0 }
				}
			};
			const SDL_MessageBoxData messageboxdata = {
				SDL_MESSAGEBOX_INFORMATION, /* .flags */
				NULL, /* .window */
				TQSE_AppTitle.c_str(), /* .title */
				message.c_str(), /* .message */
				SDL_arraysize(buttons), /* .numbuttons */
				buttons, /* .buttons */
				&colorScheme /* .colorScheme */
			};
			int buttonid;
			if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
				SDL_Log("error displaying message box");
				return;
			}
			if (buttonid == -1) {
				SDL_Log("no selection");
			} else {
				//SDL_Log("selection was %s", buttons[buttonid].text);
				SDL_Log("Selection");
			}
			return;
		}


		void Flush() {
			while (
				AppTerminate() ||
				GetKey() ||
				MouseHit(1) ||
				MouseHit(2) ||
				MouseHit(3)
				)
				Poll();
		}

		void AppTitle(std::string Title) {
			TQSE_AppTitle = Title;			
		}


		std::vector<SDL_KeyCode> KeyArray() {
			static std::vector<SDL_KeyCode> ret;
			if (!ret.size()) {
				for (int i = 0; i < numkeys; i++) ret.push_back((SDL_KeyCode)stAllKeys[i]);
			}
			return ret;
		}
	}
}
