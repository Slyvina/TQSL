// Lic:
// TQSL/Headers/TQSG_BlopPlasma.hpp
// Blop Plasma (header)
// version: 23.11.07
// Copyright (C) 2012, 2020, 2023 Jeroen P. Broks
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

/*
* This routine was originally written in BlitzMax by Imaginary Human
* In 2012 the routine was bugfixed and turned into a Module by Jeroen P. Broks
* In Dec 2020, Jeroen P. Broks manually translated this routine to C++ and linked it to TQSG, which is in turn using SDL2
* In 2023 the module was converted for the Slyvina framework by Jeroen P. Broks
*/

#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace TQSG {


		void InitBlopPlasma(JCR6::JT_Dir J, std::string entry, int NumberOfBlops = 60, int Width = 1024, int Height = 768, int BlopRadius = 511);
		void NewBlopPlasma(int NumberOfBlops = 60, int Width = 1024, int Height = 768);
		void DrawBlopPlasma(int NumBlops = 0, bool Chat = false);
		void DrawBlopPlasma(double PlasR = 1, double PlasG = 1, double PlasB = 1, int  NumBlops = 0, bool Chat = false);

		void SetPlasmaBlop(std::string File);
		void SetPlasmaBlop(JCR6::JT_Dir J, std::string Entry);
		void SetPlasmaBlop(std::string JCRFile, std::string Entry);

		// ISSUE: The internal Blop PNG doesn't work... The SetPlasmaBlop void functions are only meant to cover that bug up until this is fixed.
	}
}