// Lic:
// TQSL/Source/TQSG.cpp
// Tricky's Quick SDL2 Graphics
// version: 22.12.16
// Copyright (C) 2022 Jeroen P. Broks
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// EndLic
#include <TQSG.hpp>
#include <SlyvHSVRGB.hpp>

namespace Slyvina {
	namespace TQSG {

		static byte
			_alpha{ 255 },
			_red{ 255 },
			_green{ 255 },
			_blue{ 255 };

		void SetAlpha(byte a) { _alpha = a; }
		void SetColor(byte r, byte g, byte b) { _red = r; _green = g; _blue = b; }
		void SetColorHSV(double Hue, double Sat, double Value) {
			Units::hsv _hsv{ Hue,Sat,Value };
			auto rgb{ Units::hsv2rgb(_hsv) };
			SetColor((byte)floor(rgb.r * 255), (byte)floor(rgb.g * 255), (byte)floor(rgb.b * 255));
		}

	}
}