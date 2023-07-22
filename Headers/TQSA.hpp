// Lic:
// TQSL/Headers/TQSA.hpp
// Tricky's Quick SDL Audio (header)
// version: 23.07.22
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
#include <SDL.h>
#include <SDL_mixer.h>
#include <Slyvina.hpp>
#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace TQSA {
		class _____TAudio;
		typedef std::shared_ptr<_____TAudio> TAudio; // Shared pointer holding audio
		typedef std::unique_ptr<_____TAudio> TUAudio; // Unique pointer holding audio

		class _____TAudio {
		private:
			Mix_Chunk* ActualChunk = nullptr;
			/// <summary>
			/// Disposes the sound chunk attacked from the memory (would happen automatically if the destructor is called)
			/// </summary>
			///	void Kill();
			static uint64 count_audio;
			uint64 _id{ ++count_audio }; // debug only!
		public:

			_____TAudio(const char* File);
			_____TAudio(JCR6::JT_Dir JCRResource, std::string JCREntry);
			//_____TAudio(std::string JCRMain, std::string JCREntry);

			bool Valid();

			int Play(int loops = 0);
			void ChPlay(int channel, int loops = 0);

			//bool HasChunk();
			//bool AutoKill{ true }; // Use with CARE!
			~_____TAudio();
		};

		bool Init_TQSA(int demandflags=0);

		std::string AudioError();

		TAudio LoadAudio(std::string File);
		TUAudio LoadUAudio(std::string File);
		TAudio LoadAudio(JCR6::JT_Dir JCRResource, std::string Entry);
		TUAudio LoadUAudio(JCR6::JT_Dir JCRResource, std::string Entry);
		TAudio LoadAudio(std::string JCRResource, std::string Entry);
		TUAudio LoadUAudio(std::string JCRResource, std::string Entry);

	}

}