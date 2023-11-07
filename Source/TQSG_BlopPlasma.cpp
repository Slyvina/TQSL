// Lic:
// TQSL/Source/TQSG_BlopPlasma.cpp
// Blop Plasma
// version: 23.11.07
// Copyright (C) 2021, 2020, 2023 Jeroen P. Broks
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
#undef BlopPlasmaChat

// C++
#include <iostream>
#include <string>

// Slyvina Units
#include <SlyvRandom.hpp>

// JCR6
#include <JCR6_Core.hpp>

// TQSG
#include "../Headers/TQSG.hpp"
//#include "TQSG_BlopPlasmaBlop.hpp"
#include "../Headers/TQSG_BlopPlasma.hpp"



#ifdef BlopPlasmaChat
#define Print(a) std::cout<<a<<std::endl;
#else
#define Print(a) 
#endif

namespace Slyvina {
	namespace TQSG {

		using namespace Units;

		// Was public before. For security reasons not anymore!
		static int
			Blops = 60,
			PlasmaWidth = 1024,
			PlasmaHeight = 768;

		static const int
			Z = 512,
			S = 256,
			W = 511;

		static const double
			N = 0.2;


		// These vars were private from the start!
		static TImage
			PicBlop{ nullptr };


		static int
			C[200],
			D[200],
			E[200],
			F[200],
			E2[200],
			F2[200];

		static double
			K[200];

		void InitBlopPlasma(JCR6::JT_Dir J, std::string entry, int NumberOfBlops, int Width, int Height, int BlopRadius) {
			Print("Initiating Blop Plasma");
			//if( BlopRadius = 511) { // for now radius will ALWAYS be 511
			//Print("Blop taken from internal picture");
			//PicBlop = TQSG_LoadAutoImage(EMBER_SIZE, EMBER); //LoadImage("incbin::BlopPlasma/Blop.png")
			SetPlasmaBlop(J, entry);
			// } else { 
			// Cls
			// Print "Blop will be generated realtime"
			// Local A#, R#, G
			// For R# = 1 To BlopRadius Step N
			// A# = R * R / Width
			// SetColor Int(A), Int(A), Int(A)
			// DrawOval R / 2, R / 2, Z - R, Z - R 'change to DrawRect for inverted blobs
			// Next
			// PicBlop = CreateImage(Z, Z)
			//GrabImage(PicBlop, 0, 0)
			// EndIf
			NewBlopPlasma(NumberOfBlops, Width, Height);
			Print("Blop Plasma initiated");
		}

		void NewBlopPlasma(int NumberOfBlops, int Width, int Height) {
			//int G{ 0 };
			Blops = NumberOfBlops;
			PlasmaWidth = Width;
			PlasmaHeight = Height;
			for (int G = 0; G <= Blops - 1; G++) { // Not the best formula, but I'm trying to be as close to the original code as possible
				C[G] = Rand.Get(0, PlasmaHeight);
				D[G] = Rand.Get(0, PlasmaWidth);
				E[G] = Rand.Get(0, 4) - 2; E2[G] = abs(E[G]);
				F[G] = Rand.Get(0, 4) - 2; F2[G] = abs(F[G]);
				K[G] = C[G];
			}
		}

		void DrawBlopPlasma(int NumBlops, bool Chat) {
			DrawBlopPlasma(1, 1, 1, NumBlops, Chat);
		}

		void DrawBlopPlasma(double PlasR, double PlasG, double PlasB, int NumBlops, bool Chat) {
			std::string
				CTxt{ "" };
			int
				//G = 0,
				CX = 0,
				CY = 0,
				NBlops = Blops;			
			Blend
				Blend = GetBlend();

			//std::cout << "Test!\n";

			if (NumBlops) NBlops = NumBlops;
			Cls();
			//TQSG_SetBlend(TQSG_Blend::COLOR);//4
			SetBlend(Blend::ADDITIVE);
			for (auto G = 0; G <= NBlops - 1; G++) {// For G = 0 To NBlops - 1
				//std::cout << "G:" << G << "; C:" << C[G] << "; E:" << E[G] << "; D:" << D[G] << "; F:" << F[G] << std::endl; // Debug
				C[G] += E[G];
				D[G] += F[G];
				if (C[G] >= PlasmaWidth) { //Then
					E[G] = -E2[G];
					if (E[G] > 0) E[G] = E2[G];
				}
				if (D[G] >= PlasmaHeight) { // Then
					F[G] = -F2[G];
					if (F[G] > 0) F[G] = F2[0];
				}
				if (C[G] <= 0) {// Then
					E[G] = E2[G];
					if (E[G] < 0) E[G] = -E2[G];
				}
				if (D[G] <= 0) { // Then
					F[G] = F2[G];
					if (F[G] < 0) E[G] = -F2[G];
				}
				K[G] += N;
				if (K[G] > 360) K[G] -= 360;
				// 'SetColor (PlasmaWidth-K[G])*PlasR,(Z-C[G])*PlasG,(Z-D[G])*PlasB
				SetColor((int)(floor((double)(Z - K[G]) * PlasR)) % 256, (int)(floor((double)(Z - C[G]) * PlasG)) % 256, (int)(floor((double)(Z - D[G]) * PlasB)) % 256);
				//TQSG_Rotate(K[G]);
				//std::cout << G << ":\t Color(" << floor((double)(Z - K[G]) * PlasR) << "," << floor((double)(Z - C[G]) * PlasG) << "," << floor((double)(Z - D[G]) * PlasB) << "\n";
				//std::cout << G << ":\tPicBlop->Draw(" << C[G] << "," << D[G] << ")\n";
				PicBlop->Draw(C[G], D[G]); // DrawImage PicBlop, C[G], D[G]
				//TQSG_Circle(C[G], D[G], 100);
				if (Chat) {// Then
					//SetBlend maskblend
					// SetRotation 0
					// CTxt = "Object: " + G + " (" + C[G] + "," + D[G] + ") M(" + E[G] + "," + F[G] + ")"
					// CX = 0
					// CY = G * 15
					//While CY>PlasmaHeight - 15
					//CY:-(PlasmaHeight - 15);
				//CX:+300;
				// Wend
					// SetColor 0, 0, 0
					// DrawText CTXT, CX + 2, CY + 2
					// SetColor 255, 255, 255
					// DrawText CTXT, CX, CY
					// SetBlend 4
				}// EndIf
			}//  Next
			Rotate(0);
			SetColor(255, 255, 255);
			SetBlend(Blend);
			//PicBlop->Draw(20, 20);
		}

		void SetPlasmaBlop(std::string File) {
			PicBlop = LoadImage(File);
			PicBlop->HotCenter();
		}

		void SetPlasmaBlop(JCR6::JT_Dir J, std::string Entry) {
			PicBlop = LoadImage(J, Entry);
			PicBlop->HotCenter();
		}

		//void SetPlasmaBlop(JCR6::JT_Dir j, std::string Entry) { SetPlasmaBlop(j, Entry); }

		void SetPlasmaBlop(std::string JCRFile, std::string Entry) {
			SetPlasmaBlop(JCR6::JCR6_Dir(JCRFile), Entry);
		}



	}
}