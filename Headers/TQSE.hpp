// Lic:
// TQSL/Headers/TQSE.hpp
// Tricky's Quick SDL Event Handler (header)
// version: 23.01.17
// Copyright (C) 2022, 2023 Jeroen P. Broks
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
#include <sdl.h>
#include <Slyvina.hpp>
#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace TQSE {

		std::string AllKeys(); // A list of all known keycodes. This function only exists for debug purposes (for me).
		
		std::vector<SDL_KeyCode> KeyArray();

		typedef void (*EventFunction) (SDL_Event* Event);


		/// <summary>
		/// Polls the event and gets all data. Please note, all other readout functions will only return the data based on what this function produces
		/// </summary>
		/// <param name="EventCallBack"></param>
		void Poll(EventFunction EventCallBack = nullptr);

		/// <summary>
		/// Returns true if the last Poll had an App Termination request.
		/// </summary>
		/// <returns></returns>
		bool AppTerminate();


		/// <summary>
		/// Returns true if a key was hit during the last Poll
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		bool KeyHit(SDL_KeyCode c);

		/// <summary>
		/// Returns true if a key was held during the last Poll
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		bool KeyDown(SDL_KeyCode c);

		// Bit of a dirty way to get the pressed key and far from fully accrate, but it can sometimes be a better run.
		SDL_KeyCode GetKey();

		// Will try to get a typable character the last event. Not fully accurate, but for typing names and such it can do the job.
		unsigned char GetChar();

		int GetMouseButtons();
		int MouseX();
		int MouseY();
		void HideMouse();
		void ShowMouse();
		bool MouseDown(int code);
		bool MouseHit(int code);
		int KeyByName(std::string name);
		void ShowKeyNames();
		bool MouseReleased(int c);
		int MouseWheelY();

		bool Yes(std::string question);
		void Notify(std::string message);

		void Flush();

		/// <summary>
		/// Changes the title shown in caption bars of dialog boxes
		/// </summary>
		/// <param name="Title"></param>
		void AppTitle(std::string Title);
	}
}