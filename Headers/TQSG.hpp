// Lic:
// TQSL/Headers/TQSG.hpp
// Tricky's Quick SDL2 Graphics (header)
// version: 22.12.18
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
#include <SDL_image.h>
#include <Slyvina.hpp>
#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace TQSG {

		class _____TIMAGE; // NEVER USE THIS TYPE DIRECTLY! ONLY USE 'TImage' or 'TUImage' in stead, or you'll regret it!!!

		typedef std::shared_ptr<_____TIMAGE> TImage; // A shared pointer to use for images.
		typedef std::unique_ptr<_____TIMAGE> TUImage; // A unique pointer to use for images.


		typedef void (*TQSG_PanicType)(std::string errormessage);

		extern TQSG_PanicType TQSG_Panic; // When set the TQSG will execute this function whenever something goes wrong!

		enum class Blend {
			NONE = SDL_BLENDMODE_NONE,
			ALPHA = SDL_BLENDMODE_BLEND,
			ADDITIVE = SDL_BLENDMODE_ADD,
			COLOR = SDL_BLENDMODE_MOD
		};
		enum BlitzBlend { ALPHA = 3, ADDITIVE = 4 };


		class _____TIMAGE {
		private:
			static uint64 img_cnt;
			uint64 _ID{ ++img_cnt }; // Only serves to make debugging easier on me!
			std::vector<SDL_Texture*> Textures{};
		public:
			/// <summary>
			/// Get the number of frames
			/// </summary>
			/// <returns>The actual number of frames</returns>
			inline size_t Frames() { return Textures.size(); }

			/// <summary>
			/// Direct point to the texture used as a specific frame (please note 0 is the first frame so Frames()-1 is the last one).
			/// </summary>
			/// <param name="frame">Frame number</param>
			/// <returns>Texture pointer (nullptr if failure, although a fatal error should pop up, unless you caught that with your own panic function)</returns>						
			SDL_Texture* GetFrame(size_t frame);

			/// <summary>
			/// Disposes all frames from memory and empties the frame register. Only do this if you know what you are doing!
			/// </summary>
			void KillAllFrames();

			/// <summary>
			/// Loads data into a frame. Best not to use this unless you know what you are doing. The regular load functions will use these accordingly, so why bother?
			/// </summary>
			/// <param name="buf"></param>
			void LoadFrame(size_t frame,SDL_RWops* buf, bool autofree=true);

			/// <summary>
			/// Loads data into a new frame. Best not to use this unless you know what you are doing. The regular load functions will use these accordingly, so why bother?
			/// </summary>
			/// <param name="buf"></param>
			void LoadFrame(SDL_RWops* buf, bool autofree = true);

			void Blit(int ax, int ay, int isx, int isy, int iex, int iey, int frame = 0);
			void Blit(int x, int y, int w,int h, int isx, int isy, int iex, int iey, int frame = 0);



			int Width();
			int Height();

			inline _____TIMAGE() {} // Just to avoid some crap
			_____TIMAGE(std::string file);
			_____TIMAGE(JCR6::JT_Dir Res, std::string entry);

			~_____TIMAGE();
		};

		/// <summary>
		/// Will return the last error which happened during the last TQSG call. When the last operation went well, this will be an empty string.
		/// </summary>
		/// <returns></returns>
		std::string LastError();

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

		/// <summary>
		/// Closes graphics screen/window and frees all memory allocated for these processes. (If no graphics screen is open at the time, then simply nothing will happen at all).
		/// </summary>
		void CloseGraphics();

		/// <summary>
		/// Creates a window of the given size and height
		/// </summary>
		/// <param name="width">Window width</param>
		/// <param name="height">Window height (titlebar not counted)</param>
		/// <param name="Title">Title of the window</param>
		/// <returns>True if succesful. False if failed. LastError() will contain the error message</returns>
		bool Graphics(int width, int height, std::string Title = "Slyvina - TQSG Application");

		/// <summary>
		/// Goes into a fullscreen graphics mode.
		/// </summary>
		/// <param name="Title">Title (for what it's worth, as it's not likely to appear)</param>
		/// <returns>True if succesful. False if failed. LastError() will contain the error message</returns>
		bool Graphics(std::string Title = "Slyvina - TQSG Application");

		/// <summary>
		/// Set the CLS color. Please note this routine does not look at the current screen or window, so closing a graphics screen or window and opening a new one won't reset these values.
		/// </summary>
		/// <param name="r"></param>
		/// <param name="g"></param>
		/// <param name="b"></param>
		void SetCLSColor(byte r, byte g, byte b);

		/// <summary>
		/// Set the color of CLS, but now with HSV.
		/// </summary>
		/// <param name="Hue"></param>
		/// <param name="Saturation"></param>
		/// <param name="Value"></param>
		void SetCLSColorHSV(double, double, double);

		/// <summary>
		/// Clear the (graphics) screen
		/// </summary>
		void Cls();

		void SetBlend(Blend _blend);
		void SetBlend(SDL_BlendMode _blend);
		void SetBlitzBlend(BlitzBlend _blend);

		int ScreenWidth();
		int ScreenHeight();

		/// <summary>
		/// Drawing commands will follow alternate drawing configuration based on the settings here. This can be used to attempt to make look everything the same size regardless of the actual screen sizes. When the values are set to 0 (or a negative number) the acutal screen size will once again be used.
		/// </summary>
		/// <param name="w"></param>
		/// <param name="h"></param>
		void SetAltScreen(int w, int h);

		/// <summary>
		/// Set scale values
		/// </summary>
		/// <param name="x"></param>
		/// <param name="y"></param>
		void SetScale(double x, double y);

		/// <summary>
		/// Set scale values and both on x and y modification the same value should be used.
		/// </summary>
		/// <param name="s"></param>
		inline void SetScale(double s) { SetScale(s, s); }


		void SetOrigin(int x, int y);
		inline void SetOrigin() { SetOrigin(0, 0); }

		/// <summary>
		/// Flip will show the result of al earlier drawing requests
		/// </summary>
		/// <param name="minticks">By default Flip will wait up to 26 ticks since the last Flip, setting this parameter will change that. Please note, all changes are 'permanent' until the next change</param>
		void Flip(int minticks=-1);

		/// <summary>
		/// Draw a line
		/// </summary>
		/// <param name="start_x"></param>
		/// <param name="start_y"></param>
		/// <param name="end_x"></param>
		/// <param name="end_y"></param>
		void Line(int start_x, int start_y, int end_x, int end_y);

		/// <summary>
		/// Draw a rectangle
		/// </summary>
		/// <param name="x"></param>
		/// <param name="y"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="open"></param>
		void Rect(int x, int y, int width, int height, bool open = false);

		/// <summary>
		/// Draw a rectangle
		/// </summary>
		/// <param name="r"></param>
		/// <param name="open"></param>
		void Rect(SDL_Rect *r, bool open= false);

		/// <summary>
		/// Draw a circle
		/// </summary>
		/// <param name="center_x"></param>
		/// <param name="center_y"></param>
		/// <param name="radius"></param>
		/// <param name="segments"></param>
		void Circle(int center_x, int center_y, int radius, int segments = 200);


		/// <summary>
		/// Load an image and assigns it to a shared pointer.
		/// This routine is able to read both single picture as .jpbf files (JCR Picture Bundle File).
		/// </summary>
		/// <param name="file"></param>
		/// <returns></returns>
		TImage LoadImage(std::string file);

		/// <summary>
		/// Load an image and assigns it to a unique pointer.
		/// This routine is able to read both single picture as .jpbf files (JCR Picture Bundle File).
		/// </summary>
		/// <param name="file"></param>
		/// <returns></returns>
		TUImage LoadUImage(std::string file);

		/// <summary>
		/// Reads an image from a JCR6 resource and assigns it to a shared pointer.
		/// Pointing to a directory will cause this routine to see it as a .jpbf (JCR Picture Bundle File).
		/// </summary>
		/// <param name="J"></param>
		/// <param name="entry"></param>
		/// <returns></returns>
		TImage LoadImage(JCR6::JT_Dir J, std::string entry);

		/// <summary>
		/// Reads an image from a JCR6 resource and assigns it to a shared pointer.
		/// Pointing to a directory will cause this routine to see it as a .jpbf (JCR Picture Bundle File).
		/// </summary>
		/// <param name="J"></param>
		/// <param name="entry"></param>
		/// <returns></returns>
		TImage LoadImage(std::string JCRFile, std::string entry);

		/// <summary>
		/// Reads an image from a JCR6 resource and assigns it to a uniqur pointer.
		/// Pointing to a directory will cause this routine to see it as a .jpbf (JCR Picture Bundle File).
		/// </summary>
		/// <param name="J"></param>
		/// <param name="entry"></param>
		/// <returns></returns>
		TUImage LoadUImage(JCR6::JT_Dir J, std::string entry);

		/// <summary>
		/// Reads an image from a JCR6 resource and assigns it to a uniqur pointer.
		/// Pointing to a directory will cause this routine to see it as a .jpbf (JCR Picture Bundle File).
		/// </summary>
		/// <param name="J"></param>
		/// <param name="entry"></param>
		/// <returns></returns>
		TUImage LoadUImage(std::string JCRFile, std::string entry);
	}
}