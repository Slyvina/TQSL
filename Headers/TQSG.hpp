// Lic:
// TQSL/Headers/TQSG.hpp
// Tricky's Quick SDL2 Graphics (header)
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
#pragma once
#include <Slyvina.hpp>
#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace TQSG {

		class _____TIMAGE; // NEVER USE THIS TYPE DIRECTLY! ONLY USE 'TImage' or 'TUImage' in stead, or you'll regret it!!!

		typedef std::shared_ptr<_____TIMAGE> TImage; // A shared pointer to use for images.
		typedef std::unique_ptr<_____TIMAGE> TUImage; // A unique pointer to use for images.


		class _____TIMAGE {
		private:
		public:
		};


		/// <summary>
		/// Set the alpha value for rendering
		/// </summary>
		/// <param name="alpha"></param>
		void SetAlpha(byte);

		/// <summary>
		/// Set the color value for rendering
		/// </summary>
		/// <param name="red"></param>
		/// <param name="green"></param>
		/// <param name="blue"></param>
		void SetColor(byte, byte, byte);

		/// <summary>
		/// Set the color value and the alpha value for rendering
		/// </summary>
		/// <param name="r"></param>
		/// <param name="g"></param>
		/// <param name="b"></param>
		/// <param name="a"></param>
		inline void SetColor(byte r, byte g, byte b, byte a) { SetColor(r, g, b); SetAlpha(a); };


		/// <summary>
		/// Set color by HSV. (NOTE! Saturation and Value must be numbers from 0 to 1. Hue must be from 0 to 360)
		/// </summary>
		/// <param name="Hue">Hue</param>
		/// <param name="Sat">Saturation</param>
		/// <param name="Value">Value</param>
		void SetColorHSV(double Hue, double Sat, double Value);
	}
}