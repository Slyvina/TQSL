// License:
// 	TQSL/Source/TQSG.cpp
// 	Tricky's Quick SDL2 Graphics
// 	version: 25.01.13
// 
// 	Copyright (C) 2022, 2023, 2024, 2025 Jeroen P. Broks
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


#define TQSG_TileWithAltScreen

#undef TQSG_Debug

#ifdef TQSG_Debug
#define Chat(cht) std::cout << "\x1b[35mTQSG Debug>\t\x1b[0m" << cht <<"\n"
#else
#define Chat(cht)
#endif

#include <algorithm>

#include <TQSG.hpp>
#include <SlyvString.hpp>
#include <SlyvHSVRGB.hpp>
#include <SlyvStream.hpp>
#include <SlyvSTOI.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace TQSG {


		bool IgnoreDoubleCharError=true;


		static std::string
			_LastError{ "" };

		static byte
			_alpha{ 255 },
			_red{ 255 },
			_green{ 255 },
			_blue{ 255 },
			_clsr{ 0 },
			_clsg{ 0 },
			_clsb{ 0 };
		static double
			_scalex{ 1.0 },
			_scaley{ 1.0 },
			_rotatedeg{ 0 };
		static Blend
			_blend{ Blend::ALPHA };
		static Int32
			_originx{ 0 },
			_originy{ 0 };


		static SDL_BlendMode SDLBlend() { return (SDL_BlendMode)_blend; }


		class __Screen { // A secret class I will use to make sure SDL stuff is always properly disposed.
		public:
			SDL_Renderer* gRenderer{ nullptr };
			SDL_Window* gWindow{ nullptr };
			SDL_Surface* gScreenSurface{ nullptr };

			inline ~__Screen() {
				Chat("Closing SDL2");
				IMG_Quit();
				if (gRenderer) SDL_DestroyRenderer(gRenderer);
				if (gWindow) SDL_DestroyWindow(gWindow);
			}
		};
		std::unique_ptr<__Screen> _Screen{ nullptr };

		class __AltScreen {
		private:
			int w{ 0 };
			int h{ 0 };
			double ref_x{ 1.0 };
			double ref_y{ 1.0 };
		public:
			void SetW(int _w) {
				w = _w;
				if (w == 0)
					ref_x = 1;
				else
					ref_x = ((double)ScreenWidth(true)) / ((double)_w);
			}
			void SetH(int _h) {
				h = _h;
				if (h == 0)
					ref_y = 1;
				else
					ref_y = ((double)ScreenHeight(true)) / ((double)_h);
			}
			int GetW() { return w; }
			int GetH() { return h; }
			int X(int x) { if (w <= 0) return x; return (int)floor(x * ref_x); }
			int Y(int y) { if (h <= 0) return y; return (int)floor(y * ref_y); }
			int W(int w) { if (w <= 0) return w; return (int)ceil(w * ref_x); }
			int H(int h) { if (h <= 0) return h; return (int)ceil(h * ref_y); }
			int ScaledW(int w) { return (int)ceil(W(w) * _scalex); }
			int ScaledH(int h) { return (int)ceil(H(w) * _scalex); }
			double RX() { return ref_x; }
			double RY() { return ref_y; }
		};
		__AltScreen AltScreen;

		std::string LastError() { return _LastError; }


#pragma region Paniek
		TQSG_PanicType TQSG_Panic{ nullptr };
		static void DefaultPanic(std::string str) {
			SDL_Window* Win{ nullptr };
			if (_Screen) Win = _Screen->gWindow;
			std::cout << "\x1b[31mTQSG went into a panic!\x1b[0m\n" << str << std::endl;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "TQSG - Fatal error!", str.c_str(), Win);
			exit(255);
		}
		static void Paniek(std::string errmsg) {
			if (!TQSG_Panic) TQSG_Panic = DefaultPanic;
			TQSG_Panic(errmsg);
			_LastError = "FATAL ERROR: " + errmsg;
		}
		inline bool NeedScreen() { if (!_Screen) { Paniek("Action requiring a graphics screen"); return false; } else return true; }
#pragma endregion

#pragma region GeneralCommands
		int ASY(int y) { return AltScreen.Y(y); }
		int ASX(int x) { return AltScreen.X(x); }
		void SetAlpha(byte a) { _alpha = a; _LastError = ""; }
		void SetAlphaD(double a) {
			a = std::min(a, (double)1); a = std::max((double)0, a);
			SetAlpha((byte)floor(a * 255));
		}
		void SetColor(byte r, byte g, byte b) { _red = r; _green = g; _blue = b; _LastError = ""; }
		void GetColor(Byte& R, Byte& G, Byte& B) { R = _red; G = _green; B = _blue; _LastError = ""; }
		void SetColorHSV(double Hue, double Sat, double Value) {
			Units::hsv _hsv{ Hue, Sat, Value };
			auto rgb{ Units::hsv2rgb(_hsv) };
			SetColor((byte)floor(rgb.r * 255), (byte)floor(rgb.g * 255), (byte)floor(rgb.b * 255));
		}

		void CloseGraphics() { _Screen = nullptr; }

		inline bool NeedSDL() {
			static bool Done{ false };
			if (Done) return true;
			Chat("Init SDL2");
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
				_LastError = TrSPrintF("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
				Chat(_LastError);
				_Screen = nullptr;
				return false; //success = false;
			} else {
				Chat("=> Success");
				Done = true;
				return true;
			}
		}

		static bool TrueGraphics(int width, int height, bool fullscreen, std::string Title) {
			Chat(TrSPrintF("Starting graphics screen: %dx%d; fullscreen=%d\n", width, height, fullscreen));
			_LastError = "";
			_Screen = std::make_unique<__Screen>();

			//Initialize SDL
			if (NeedSDL()) {
				//Create window
				_Screen->gWindow = SDL_CreateWindow(Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
				if (_Screen->gWindow == NULL) {
					_LastError = TrSPrintF("Window could not be created! SDL_Error: %s\n", SDL_GetError());
					Chat(_LastError);
					_Screen = nullptr;
					return false;//success = false;
				} else {
					if (fullscreen) {
						int fcsuc = SDL_SetWindowFullscreen(_Screen->gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
						if (fcsuc != 0) {
							printf("\a\x1b[33mWARNING!\x1b[0m\t Going into full screen was unsuccesful\n\n");
						}
					}
					_Screen->gRenderer = SDL_CreateRenderer(_Screen->gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
					if (_Screen->gRenderer == NULL) {
						_LastError = TrSPrintF("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
						Chat(_LastError);
						//SDL_DestroyWindow(gWindow);
						_Screen = nullptr;
						return false;
						// success = false;
					} else {
						//Initialize renderer color
						SDL_SetRenderDrawColor(_Screen->gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

						//Initialize PNG loading
						int imgFlags = IMG_INIT_PNG;
						if (!(IMG_Init(imgFlags) & imgFlags)) {
							printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
							_LastError = "SDL_Image: " + std::string(IMG_GetError());
							//SDL_DestroyWindow(gWindow);
							//SDL_DestroyRenderer(gRenderer);
							_Screen = nullptr;
							return false;

							//success = false;
						}
					}

					//Get window surface
					_Screen->gScreenSurface = SDL_GetWindowSurface(_Screen->gWindow);
					if (!_Screen->gScreenSurface) printf("Creating a surface failed\n");

					// Load Death
					//printf("Loading Death Picture\n");
					//Tex_Death = Tex_From_JCR(ARF, "Pics/Death.png");
#ifdef TQSG_AllowTTF
					Chat("Init TTF");
					if (TTF_Init() == -1) {
						printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
						return false;
					}
#endif
					Cls();
					return true;
				}
			}
			return false;
		}

		bool Graphics(int width, int height, std::string Title) { return TrueGraphics(width, height, false, Title); }
		bool Graphics(std::string Title) { return TrueGraphics(0, 0, true, Title); }

		void SetCLSColor(byte r, byte g, byte b) {
			_clsr = r;
			_clsg = g;
			_clsb = b;
		}

		void SetCLSColorHSV(double h, double s, double v) {
			hsv sr{ h,s,v };
			rgb t{ hsv2rgb(sr) };
			SetCLSColor((byte)floor(t.r * 255), (byte)floor(t.g * 255), (byte)floor(t.b * 255));
		}

		void Cls() {
			Uint8  r, g, b, a;
			_LastError = "";
			if (!_Screen) {
				_LastError = "CLS(): Impossible to comply without a graphics screen";
				return;
			}
			SDL_GetRenderDrawColor(_Screen->gRenderer, &r, &g, &b, &a);
			SDL_SetRenderDrawColor(_Screen->gRenderer, _clsr, _clsg, _clsb, 255);
			SDL_RenderClear(_Screen->gRenderer);
			SDL_SetRenderDrawColor(_Screen->gRenderer, r, g, b, a);
		}

		void SetBlend(Blend _argblend) { _blend = _argblend; _LastError = ""; }
		void SetBlend(SDL_BlendMode _argblend) { _blend = (Blend)_argblend; _LastError = ""; }
		Blend GetBlend() { return _blend; }

		void SetBlitzBlend(BlitzBlend i) {
			/*
			Const MASKBLEND=1
			Const SOLIDBLEND=2
			Const ALPHABLEND=3
			Const LIGHTBLEND=4
			Const SHADEBLEND=5
			*/
			_LastError = "";
			switch (i) {
			case 0:
			case 3:
				SetBlend(Blend::ALPHA);
				break;
			case 4:
				SetBlend(Blend::ADDITIVE);
				break;
			default:

				_LastError = "ERROR! Unknown blitz blend (" + std::to_string(i) + ")";
				Chat(_LastError);
				break;
			}
		}

		int ScreenWidth(bool pure) {
			int w, h;
			_LastError = "";
			if (!NeedScreen()) return 0;
			SDL_GetRendererOutputSize(_Screen->gRenderer, &w, &h);
			if (pure || AltScreen.GetW() == 0)
				return w;
			else
				return AltScreen.GetW();
		}
		int ScreenHeight(bool pure) {
			int w, h;
			_LastError = "";
			if (!NeedScreen()) return 0;
			SDL_GetRendererOutputSize(_Screen->gRenderer, &w, &h);
			if (pure || AltScreen.GetH() == 0)
				return h;
			else
				return AltScreen.GetH();
		}

		void SetAltScreen(int w, int h) {
			AltScreen.SetW(w);
			AltScreen.SetH(h);
		}

		void SetScale(double w, double h) {
			_scalex = w;
			_scaley = h;
		}

		void SetOrigin(int x, int y) {
			_originx = x;
			_originy = y;
		}

		void WaitMinTicks(int minticks) {
			if (!NeedScreen()) return;
			//SDL_UpdateWindowSurface(gWindow);
			static auto oud{ SDL_GetTicks() };
			static auto mt{ 26 };
			if (minticks >= 0) mt = minticks;
			while (minticks && (SDL_GetTicks() - oud < mt)) SDL_Delay(1);
			oud = SDL_GetTicks();
		}

		void Flip(int minticks) {
			WaitMinTicks(minticks);
			SDL_RenderPresent(_Screen->gRenderer);
		}

		void Line(int start_x, int start_y, int end_x, int end_y) {
			if (!NeedScreen()) return;
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			SDL_RenderDrawLine(_Screen->gRenderer, start_x, start_y, end_x, end_y);
		}


		void ALine(int start_x, int start_y, int end_x, int end_y) {
			if (!NeedScreen()) return;
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			SDL_RenderDrawLine(_Screen->gRenderer, AltScreen.X(start_x), AltScreen.Y(start_y), AltScreen.X(end_x), AltScreen.Y(end_y));
		}

		void Rect(int x, int y, int width, int height, bool open) {
			SDL_Rect r{ x,y,width,height };
			Rect(&r, open);
		}

		void ExRect(int sx, int sy, int ex, int ey) {
			auto
				startx{ std::min(sx,ex) },
				starty{ std::min(sy,ey) },
				endx{ std::max(sx,ex) },
				endy{ std::max(sy,ey) };

			Rect(startx, starty, endx - startx, endy - starty);
		}

		void Rect(SDL_Rect* r, bool open) {
			if (!NeedScreen) return;
			SDL_SetRenderDrawBlendMode(_Screen->gRenderer, (SDL_BlendMode)_blend);
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			if (open)
				SDL_RenderDrawRect(_Screen->gRenderer, r);
			else
				SDL_RenderFillRect(_Screen->gRenderer, r);

		}

		void ARect(int x, int y, int w, int h, bool open) {
			Rect(AltScreen.X(x), AltScreen.Y(y), AltScreen.W(w), AltScreen.H(h), open);
		}

		void ACircle(int center_x, int center_y, int radius, int segments) {
			static double doublepi{ 2 * 3.14 };
			double progress{ doublepi / (double)std::max(segments,4) };
			float lastx = center_x, lasty = (radius)+center_y, firstx = lastx, firsty = lasty;
			for (double i = 0; i < 2 * 3.14; i += progress) {
				float cx = (sin(i) * radius) + center_x, cy = (cos(i) * radius) + center_y;
				//SDL_RenderDrawLine(gRenderer, lastx, lasty, cx, cy);
				Line(AltScreen.X((int)lastx), AltScreen.Y((int)lasty), AltScreen.X((int)cx), AltScreen.Y((int)cy));
				lastx = cx; lasty = cy;
			}
			Line(AltScreen.X((int)lastx), AltScreen.Y((int)lasty), AltScreen.X((int)firstx), AltScreen.Y((int)firsty)); // Make sure the final segment is drawn as well.
		}

		void Circle(int center_x, int center_y, int radius, int segments) {
			static double doublepi{ 2 * 3.14 };
			double progress{ doublepi / (double)std::max(segments,4) };
			float lastx = center_x, lasty = (radius)+center_y, firstx = lastx, firsty = lasty;
			for (double i = 0; i < 2 * 3.14; i += progress) {
				float cx = (sin(i) * radius) + center_x, cy = (cos(i) * radius) + center_y;
				//SDL_RenderDrawLine(gRenderer, lastx, lasty, cx, cy);
				Line((int)lastx, (int)lasty, (int)cx, (int)cy);
				lastx = cx; lasty = cy;
			}
			Line(lastx, lasty, firstx, firsty); // Make sure the final segment is drawn as well.
		}

		TImage LoadImage(std::string file) {
			_LastError = "";
			if (!FileExists(file)) {
				_LastError = "LoadImage(" + file + "): File not found!";
				return nullptr;
			}
			auto ret{ std::make_shared<_____TIMAGE>(file) };
			if (_LastError.size()) return nullptr;
			return ret;
		}

		TUImage LoadUImage(std::string file) {
			_LastError = "";
			if (!FileExists(file)) {
				_LastError = "LoadImage(" + file + "): File not found!";
				return nullptr;
			}
			auto ret{ new _____TIMAGE(file) };
			if (_LastError.size()) {
				delete ret;
				return nullptr;
			}
			return std::unique_ptr<_____TIMAGE>(ret);
		}

		TImage LoadImage(JCR6::JT_Dir J, std::string entry) {
			_LastError = "";
			auto ret{ new _____TIMAGE(J,entry) };
			if (_LastError.size()) { delete ret; return nullptr; }
			if (!ret->Frames()) {
				std::cout << "Trying to load entry " << entry << " resulted into zero frames!";
				std::cout << "JCR6: " << JCR6::Last()->ErrorMessage << " (" << JCR6::Last()->MainFile << "::" << JCR6::Last()->Entry << ")\n";
				return nullptr;
			}
			return std::shared_ptr<_____TIMAGE>(ret);
		}

		TImageFont LoadImageFont(JCR6::JT_Dir Res, std::string path) {
			_LastError = "";
			return std::shared_ptr<_____TIMAGEFONT>(new _____TIMAGEFONT(Res, path));
		}

		TUImageFont LoadUImageFont(JCR6::JT_Dir Res, std::string path) {
			_LastError = "";
			return std::unique_ptr<_____TIMAGEFONT>(new _____TIMAGEFONT(Res, path));
		}

		TImageFont LoadImageFont(std::string JCRRes, std::string path) {
			_LastError = "";
			auto J = JCR6::JCR6_Dir(JCRRes);
			if (JCR6::Last()->Error) { _LastError = "LoadImageFont(\"" + JCRRes + "\", \"" + path + "\"): " + JCR6::Last()->ErrorMessage; return nullptr; }
			return std::shared_ptr<_____TIMAGEFONT>(new _____TIMAGEFONT(J, path));
		}

		TUImageFont LoadUImageFont(std::string JCRRes, std::string path) {
			_LastError = "";
			auto J = JCR6::JCR6_Dir(JCRRes);
			if (JCR6::Last()->Error) { _LastError = "LoadImageFont(\"" + JCRRes + "\", \"" + path + "\"): " + JCR6::Last()->ErrorMessage; return nullptr; }
			return std::unique_ptr<_____TIMAGEFONT>(new _____TIMAGEFONT(J, path));
		}


		void Plot(int x, int y) {
			_LastError = "";
			if (!NeedScreen()) return;
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			SDL_RenderDrawPoint(_Screen->gRenderer, x, y);
		}

		int32 DesktopWidth(bool panic) {
			_LastError = "";
			SDL_DisplayMode mode;
			if (!NeedSDL()) return 0;
			if (!SDL_GetDesktopDisplayMode(0, &mode)) _LastError = SDL_GetError();
			if (panic && _LastError.size()) Paniek(_LastError);
			return mode.w;
		}

		int32 DesktopHeight(bool panic) {
			_LastError = "";
			SDL_DisplayMode mode;
			if (!NeedSDL()) return 0;
			if (!SDL_GetDesktopDisplayMode(0, &mode)) _LastError = SDL_GetError();
			if (panic && _LastError.size()) Paniek(_LastError);
			return mode.h;
		}

		void TQSG_DesktopSize(int& w, int& h) {
			SDL_DisplayMode mode;
			SDL_GetDesktopDisplayMode(0, &mode);
			w = mode.w;
			h = mode.h;
		}



		TImage LoadImage(std::string JCRFile, std::string entry) {
			_LastError = "";
			auto J{ JCR6::JCR6_Dir(JCRFile) };
			if (JCR6::Last()->Error) { _LastError = JCR6::Last()->ErrorMessage;  return nullptr; }
			return LoadImage(J, entry);
		}

		TUImage LoadUImage(JCR6::JT_Dir J, std::string entry) {
			auto ret{ new _____TIMAGE(J,entry) };
			if (_LastError.size()) { delete ret; return nullptr; }
			if (!ret) {
				std::cout << "ERROR! Entry '" << entry << "' resulted to null. Cannot create a unique pointer!\n";
				return nullptr;
			}
			return std::unique_ptr<_____TIMAGE>(ret);
		}

		TUImage LoadUImage(std::string JCRFile, std::string entry) {
			_LastError = "";
			auto J{ JCR6::JCR6_Dir(JCRFile) };
			if (JCR6::Last()->Error) { _LastError = JCR6::Last()->ErrorMessage;  return nullptr; }
			auto ret{ new _____TIMAGE(J,entry) };
			if (_LastError.size()) { delete ret; return nullptr; }
			return std::unique_ptr<_____TIMAGE>(ret);
		}

		void SetMouse(int x, int y) {
			if (!NeedScreen) return;
			SDL_WarpMouseInWindow(_Screen->gWindow, x, y);
		}

		void Rotate(double degrees) { _rotatedeg = degrees; }
#pragma endregion

#pragma region TQAltPic
		bool TQAltPic::_indexed{ false };
		std::map<std::string, TQAltPic*> TQAltPic::_ExtIndex{};
		std::vector<TQAltPic> TQAltPic::_List{};

		void TQAltPic::ReIndex(bool onlywhenneeded) {
			if ((!_indexed) || (!onlywhenneeded)) {
				_ExtIndex.clear();
				for (auto& ei : _List) {
					if (ei._Ext.size()) {
						auto eis{ Split(ei._Ext,';') };
						for (auto eise:*eis) _ExtIndex[eise] = &ei;
					}
				}
			}
			_indexed = true;
		}
#pragma endregion

#pragma region TImage

		SDL_Texture* _____TIMAGE::GetFrame(size_t frame) {
			if (frame >= Frames()) { Paniek(TrSPrintF("Frame exceeeds max. (%d) (there are %d frames)", frame, Frames())); return nullptr; }
			return Textures[frame];
		}

		void _____TIMAGE::KillAllFrames() {
			for (auto T : Textures) SDL_DestroyTexture(T);
			Textures.clear();
		}

		void _____TIMAGE::LoadFrame(size_t frame, SDL_RWops* data, bool autofree) {
			_LastError = "";
			if (frame >= Textures.size()) {
				//char FE[400];
				//sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				//_LastError = FE;
				_LastError = TrSPrintF("Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				return;
			}
			if (!NeedScreen()) return;
			auto buf = IMG_LoadTexture_RW(_Screen->gRenderer, data, autofree);

			if (buf == NULL) { Paniek("Getting texture from SDL_RWops failed!"); return; }
			Textures[frame] = buf;
		}
		void _____TIMAGE::LoadFrame(SDL_RWops* data, bool autofree) {
			_LastError = "";
			if (!NeedScreen()) return;
			auto buf = IMG_LoadTexture_RW(_Screen->gRenderer, data, autofree);
			if (buf == NULL) { Paniek("Getting texture from SDL_RWops failed!"); return; }
			Textures.push_back(buf);
		}

		void _____TIMAGE::Blit(int ax, int ay, int isx, int isy, int iex, int iey, int frame) {
			if (!NeedScreen()) return;
			auto
				x = ax + _originx,
				y = ay + _originy,
				imgh = Height(),
				imgw = Width();
			SDL_Rect
				Target,
				Source;
			//if (altframing) {
			//	LastError = "Altframing NOT supported for Blitting (and is not likely to be supported in the future, either!)";
			//	return;
			//}
			Source.x = std::max(isx, 0);
			Source.y = std::max(isy, 0);
			Source.w = std::min(iex, imgw - isx);
			Source.h = std::min(iey, imgh - isy);
			if (Source.w < 1 || Source.h < 1) { _LastError = "Blit format error"; return; }
			/*
			Target.x = x;
			Target.y = y;
			Target.w = Source.w * scalex;
			Target.h = Source.h * scaley;
			//*/
			Target.x = AltScreen.X(x);
			Target.y = AltScreen.Y(y);
			Target.w = AltScreen.ScaledW(Source.w);
			Target.h = AltScreen.ScaledW(Source.h);
			SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
			SDL_SetTextureBlendMode(Textures[frame], (SDL_BlendMode)_blend);
			SDL_SetTextureAlphaMod(Textures[frame], _alpha);
			SDL_RenderCopy(_Screen->gRenderer, Textures[frame], &Source, &Target);
		}
		void _____TIMAGE::Blit(int ax, int ay, int w, int h, int isx, int isy, int iex, int iey, int frame) {
			if (!NeedScreen()) return;
			auto
				x = ax + _originx,
				y = ay + _originy,
				imgh = Height(),
				imgw = Width();
			SDL_Rect
				Target,
				Source;
			//if (altframing) {
			//	LastError = "Altframing NOT supported for Blitting (and is not likely to be supported in the future, either!)";
			//	return;
			//}
			Source.x = std::max(isx, 0);
			Source.y = std::max(isy, 0);
			Source.w = std::min(iex, imgw - isx);
			Source.h = std::min(iey, imgh - isy);
			if (Source.w < 1 || Source.h < 1) { _LastError = "Blit format error"; return; }
			Target.x = AltScreen.X(x);
			Target.y = AltScreen.Y(y);
			Target.w = AltScreen.W(w);
			Target.h = AltScreen.H(h);
			SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
			SDL_SetTextureBlendMode(Textures[frame], (SDL_BlendMode)_blend);
			SDL_SetTextureAlphaMod(Textures[frame], _alpha);
			SDL_RenderCopy(_Screen->gRenderer, Textures[frame], &Source, &Target);

		}

		int _____TIMAGE::Width() {
			_LastError = "";
			if (AltPic && AltPic->Width) return AltPic->Width(this);
			if (!Frames()) {
				_LastError = "<Image>->Width(): No Frames";
				return 0;
			}
			int w, h;
			SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
			return w;
		}

		int _____TIMAGE::Height() {
			_LastError = "";
			if (AltPic && AltPic->Height) return AltPic->Height(this);
			if (!Frames()) {
				_LastError = "<Image>->Height(): No Frames";
				return 0;
			}
			int w, h;
			SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
			return h;
		}

		void _____TIMAGE::GetFormat(int* width, int* height) {
			_LastError = "";
			if (AltPic && AltPic->GetFormat) { AltPic->GetFormat(this, width, height); return; }
			if (!Frames()) {
				_LastError = "<Image>->Height(): No Frames";
				return;
			}
			SDL_QueryTexture(Textures[0], NULL, NULL, width, height);
		}

		uint64 _____TIMAGE::img_cnt{ 0 };
		_____TIMAGE::_____TIMAGE(std::string file) {
			auto fext{ Upper(ExtractExt(file)) };
			auto tqae{ TQAltPic::ExtDriver(fext) };
			if (tqae && tqae->LoadReal) {
				tqae->LoadReal(this, file);
				AltPic = tqae;
				return;
			}
			Chat("Image " << _ID << " created by file " << file);
			if (!NeedScreen()) return;
			_LastError = "";
			if (JCR6::_JT_Dir::Recognize(file) != "NONE") {
				auto J{ JCR6::JCR6_Dir(file) };
				auto E{ J->Entries() };
				for (auto Ent : *E) {
					auto ext{ Upper(ExtractExt(Ent->Name())) };
					if (ext == "PNG" || ext == "BMP" || ext == "JPG" || ext == "JPEG") {
						auto jbuf{ J->B(Ent->Name()) };
						if (JCR6::Last()->Error) {
							_LastError = "JCR6 Error: " + JCR6::Last()->ErrorMessage + "\n (File: " + file + "; Entry:" + Ent->Name() + ")";
							return;
						}
						auto buf{ SDL_RWFromMem(jbuf->Direct(),Ent->RealSize()) };

						LoadFrame(buf);
					}
				}
			} else {
				auto surf{ IMG_Load(file.c_str()) };
				if (surf == NULL) {
					//char FE[300];
					//sprintf_s(FE, 295, "Unable to load image %s!\nSDL_image Error: %s", file.c_str(), IMG_GetError());
					//_LastError = FE;
					_LastError = "Unable to load image "+file+"!\nSDL_image Error : " + IMG_GetError();
					return;
				}
				//Create texture from surface pixels
				auto newTexture = SDL_CreateTextureFromSurface(_Screen->gRenderer, surf);
				if (newTexture == NULL) {
					//char FE[300];
					//sprintf_s(FE, 295, "Unable to create texture from %s!\nSDL Error: %s", file.c_str(), SDL_GetError());
					//_LastError = FE;
					_LastError = TrSPrintF("Unable to create texture from %s!\nSDL Error: ", file.c_str()) + SDL_GetError();
					return;
				}
				Textures.push_back(newTexture);
			}
		}

		_____TIMAGE::_____TIMAGE(JCR6::JT_Dir Res, std::string entry) {
			Chat("Image " << _ID << " created by JCR resource (entry " << entry << ")");
			_LastError = "";
			if (!Res) { Paniek("Trying to get images from null!"); return; }
			if (!NeedScreen()) return;
			auto fext{ Upper(ExtractExt(entry)) };
			auto tqae{ TQAltPic::ExtDriver(fext) };
			if (tqae && tqae->LoadJCR6) {
				Frames();
				tqae->LoadJCR6(this, Res,entry);
				AltPic = tqae;
				return;
			}
			//std::cout << entry << " entry(" << Res->EntryExists(entry) << ") dir(" << Res->DirectoryExists(entry) << ")\n"; // debug
			if (Res->EntryExists(entry)) {
				auto buf{ Res->B(entry) };
				if (JCR6::Last()->Error) {
					_LastError = "JCR6 Error: " + JCR6::Last()->ErrorMessage + "\n (Entry:" + entry + ")";
					return;
				}
				auto _rwops{ SDL_RWFromMem(buf->Direct(),buf->Size()) };
				LoadFrame(_rwops);
			} else if (Res->DirectoryExists(entry)) {
				auto files{ Res->Directory(entry) };
				for (auto f : *files) {
					auto ext{ Upper(ExtractExt(f)) };
					//std::cout << entry << " -> " << f << std::endl; // debug
					if (ext == "PNG" || ext == "JPG" || ext == "JPEG" || ext == "BMP") {
						auto buf{ Res->B(f) };
						if (JCR6::Last()->Error) {
							_LastError = "JCR6 Error: " + JCR6::Last()->ErrorMessage + "\n (Entry:" + f + " of sequence " + entry + ")";
							return;
						}
						auto _rwops{ SDL_RWFromMem(buf->Direct(),buf->Size()) };
						LoadFrame(_rwops);
					}
				}
			}
			//std::cout << "LoadImage " << entry << " Textures:" << Textures.size() << std::endl; // debug
			if (!Textures.size()) { _LastError = "No textures loaded!"; return; }
			auto hotfile{ StripExt(entry) + ".hot" }; //std::cout << "HOT!!! " << hotfile << " exists: " << boolstring(Res->EntryExists(hotfile)) << "\n";
			if (Res->EntryExists(hotfile)) {
				auto hotcontent{ Upper(Trim(Res->GetString(hotfile))) };
				//std::cout << "\7HOT!!! " << hotcontent; for (size_t i = 0; i < hotcontent.size(); ++i) std::cout << "; Chr(" << (int)hotcontent[i] << ")"; std::cout << "\n";
				if (hotcontent == "CENTER") {
					HotCenter();
					//std::cout << "HOT!!! Centered\n";
				} else if (hotcontent == "BOTTOMCENTER") HotBottomCenter();
				else {
					auto p{ FindFirst(hotcontent,",") }; if (p < 0) { _LastError = "Hotspot error"; return; }
					auto hx{ ToInt(hotcontent.substr(0,p)) };
					auto hy{ ToInt(hotcontent.substr(p + 1)) };
					Hot(hx, hy);
				}
			}
		}


		_____TIMAGE::~_____TIMAGE() {
			Chat("Image " << _ID << " destroyed");
			if (AltPic && AltPic->Destroy) {
				AltPic->Destroy(this); return;
			}
			KillAllFrames();
		}

		void _____TIMAGE::StretchDraw(int x, int y, int w, int h, int frame) {
			if (!NeedScreen()) return;
			if (!this) throw std::runtime_error(TrSPrintF("<NULL>.StretchDraw(%d,%d,%d,%d,%d): Can't stretch from null!", x, y, w, h, frame));
			_LastError = "";
			if (frame < 0 || frame >= Textures.size()) {
				//char FE[400];
				//sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				//_LastError = FE;
				_LastError = TrSPrintF("Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				return;
			}
			SDL_Rect Target;
			Target.x = AltScreen.X(x + _originx);
			Target.y = AltScreen.Y(y + _originy);
			Target.w = AltScreen.W(w);
			Target.h = AltScreen.H(h);
			SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
			SDL_SetTextureAlphaMod(Textures[frame], _alpha);
			SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
			SDL_RenderCopy(_Screen->gRenderer, Textures[frame], NULL, &Target);
		}

		void _____TIMAGE::Draw(int x, int y, int frame) {
			_LastError = "";
			if (!this){_LastError=TrSPrintF("<nullpointer>.Draw(%d,%d,%d)",x,y,frame); Paniek(_LastError); }
			if (!NeedScreen()) return;
			if (AltPic && AltPic->Draw) { AltPic->Draw(this, x, y, frame); return; }
			if (frame < 0 || frame >= Textures.size()) {
				//char FE[400];
				//sprintf_s(FE, 395, "DRAW:Texture frame assignment out of bouds! (%d/%d/R)", frame, (int)Textures.size());
				//LastError = FE;
				auto FE{ TrSPrintF("DRAW:Texture frame assignment out of bouds! (%d/%d/R)", frame, (int)Textures.size()) };
				Paniek(FE);
				return;
			}
			//std::cout << "...\n";
			SDL_Rect Target;
			Target.x = AltScreen.X((x - (int)ceil(hotx * _scalex)) + _originx);
			Target.y = AltScreen.Y((y - (int)ceil(hoty * _scaley)) + _originy);
			Target.w = AltScreen.W((int)ceil(Width() * _scalex));
			Target.h = AltScreen.H((int)ceil(Height() * _scaley));
			SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
			SDL_SetTextureAlphaMod(Textures[frame], _alpha);
			SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
			SDL_RenderCopy(_Screen->gRenderer, Textures[frame], NULL, &Target);
		}

		void _____TIMAGE::TrueDraw(int x, int y, int frame) {
			_LastError = "";
			if (!NeedScreen()) return;
			if (frame < 0 || frame >= Textures.size()) {
				/*
				char FE[400];
				sprintf_s(FE, 395, "DRAW:Texture frame assignment out of bouds! (%d/%d/R)", frame, (int)Textures.size());
				//LastError = FE;
				Paniek(FE);
				//*/
				Paniek(TrSPrintF("DRAW:Texture frame assignment out of bouds! (%d/%d/R)", frame, (int)Textures.size()));
				return;
			}
			SDL_Rect Target;
			Target.x = x - hotx; //+_originx;
			Target.y = y - hoty; //+_originy;
			Target.w = Width();
			Target.h = Height();
			SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
			SDL_SetTextureAlphaMod(Textures[frame], _alpha);
			SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
			SDL_RenderCopy(_Screen->gRenderer, Textures[frame], NULL, &Target);
		}

		void _____TIMAGE::XDraw(int x, int y, int frame) {
			if (!NeedScreen()) return;
			int limgflip{ SDL_FLIP_NONE };
			auto _scalex{ TQSG::_scalex };
			auto _scaley{ TQSG::_scaley };
			_LastError = "";
			if (frame < 0 || frame >= Textures.size()) {
				/* Old code, and gcc doesn't like sprintf_s
				char FE[400];
				sprintf_s(FE, 395, "XDRAW:Texture frame assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				_LastError = FE;
				*/
				auto FE = TrSPrintF("XDRAW:Texture frame assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				Paniek(FE);
				return;
			}
			if (_scalex < 0) {
				_scalex = abs(_scalex);
				limgflip |= SDL_FLIP_HORIZONTAL;
			}
			if (_scaley < 0) {
				_scaley = abs(_scaley);
				limgflip |= SDL_FLIP_VERTICAL;
			}
			//*
				SDL_Rect Target{
					AltScreen.X((x - (hotx * _scalex)) + _originx),
					AltScreen.Y((y - (hoty * _scaley)) + _originy),
					AltScreen.W((int)(Width() * _scalex)),
					AltScreen.H((int)(Height() * _scaley))
			};

			SDL_Point cpoint{ (int)(hotx * _scalex * AltScreen.RX()),(int)(hoty * _scaley * AltScreen.RY()) };

			//SDL_RenderCopy(gRenderer, Textures[frame], NULL, &Target);
				SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
				SDL_SetTextureAlphaMod(Textures[frame], _alpha);
				SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
				SDL_RenderCopyEx(_Screen->gRenderer, Textures[frame], NULL, &Target, _rotatedeg, &cpoint, (SDL_RendererFlip)limgflip);

		}

		void _____TIMAGE::Tile(int ax, int ay, int w, int h, int frame, int aix, int aiy) {
			using namespace std;
			if (!NeedScreen()) return;
			try {
#ifdef TQSG_TileWithAltScreen
				auto
					x = ax + _originx,
					y = ay + _originy,
					ix = aix,
					iy = aiy;


				_LastError = "";
				// todo: Fix issues with negative ix
				/*???
				if (iy>0)
					iy = (y + (Height() - iy)) % Height();
				if (ix > 0)
					//ix = (x+ (Width() - ix)) % Width();
					//ix = (x - (Width() + ix)) % Width();
					ix = -(ix % Width());
					//*/
				if (ix < 0) {
					//cout << "neg x:" << ix << " to ";
					//ix = (AltScreen.X(x) - (Width() + ix)) % Width();
					//cout << ix << "\n";

					// Faulty: 	ix = (x - (Width() + ix)) % Width();
					ix = Width() - (abs(ix) % Width());
				}
				if (iy < 0) {
					//cout << "neg x:" << ix << " to ";
					//iy = (AltScreen.Y(y) - (Height() + iy)) % Height();
					//cout << ix << "\n";

					// Faulty: iy = (y - (Height() + iy)) % Height();
					iy = Height() - (abs(iy) % Height());
				}
				//int ox, oy, ow, oh;
				//TQSG_GetViewPort(&ox, &oy, &ow, &oh);
				int tsx, tsy, tex, tey, tw, th;
				int imgh = Height();
				int imgw = Width();
				/*
				tsx = max(ox, x);
				tsy = max(oy, y);
				tex = min(ow + ox, x + w); tw = tex - tsx;
				tey = min(oh + oy, y + h); th = tey - tsy;
				*/
				tsx = x;
				tsy = y;
				tex = w + x;
				tey = h + y;
				tw = w;
				th = h;
				if (tw <= 0 || th <= 0) return; // Nothing to do but getting bugged!
				//cout << "TILE: Rect("<<x<<","<<y<<") "<<w<<"x"<<h<<" "<<"\n";
				//cout << "\tViewPort(" << tsx << "," << tsy << "," << tw << "[" << tex << "]" << "," << th << "[" << tey << "])\n";
				SDL_Rect Target, Source;
				//TQSG_ViewPort(tsx, tsy, tw, th);
				//TQSG_Rect(tsx, tsy, tw, th);
				//cout << "for (int dy = tsy("<<tsy<<") - iy("<<iy<<")(" << (tsy - iy) << "); dy < tey(" << tey << "); dy += imgh(" << imgh << ")) \n";
				//cout << "Color (" << (int)_red << "," << (int)_green << "," << (int)_blue << ")\n"; // DEBUG
				//printf("TImage::Tile(%d,%d,%d,%d,%d,%d,%d):\n", ax, ay, w, h, frame, aix, aiy); // DEBUG
				SDL_SetTextureColorMod(Textures[frame], _red, _green, _blue);
				//cout << "Blend " << SDLBlend() << "\n"; // DEBUG
				SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
				SDL_SetTextureAlphaMod(Textures[frame], _alpha);
				for (int dy = tsy - iy; dy < tey; dy += imgh) {
					//cout << "(" << x << "," << y << ")\tdy:" << dy << "; tsy:" << tsy << " imgh:" << imgh << " th:" << th << "\n";
					for (int dx = tsx - ix; dx < tex; dx += imgw) {
						//cout << "\t\tDrawTile(" << dx << "," << dy << "," << imgw << "," << imgh << ")\n";
						Target.x = dx;
						Target.y = dy;
						Target.w = imgw;
						Target.h = imgh;
						Source.x = 0;
						Source.y = 0;
						Source.w = imgw;
						Source.h = imgh;
						//cout << "tgt (" << Target.x << "," << Target.y << ") " << Target.w << "x" << Target.h<<"\n";
						//cout << "src (" << Source.x << "," << Source.y << ") " << Source.w << "x" << Source.h<<"; Frame:"<<frame<<"\n\n";
						//cout << "("<<x<<","<<y<<")\tdx:" << dx << "; tsx:" << tsx << " imgw:" << imgw << " tw:" << tw<<"\n";
						if (dx >= tsx && (dx + imgw) > tex) {
							Source.w = imgw - ((dx + imgw) - tex);
							Target.w = Source.w; //(dx + imgw) - tex;
							//cout << "aw " << Source.w << "\n";
						} else if (dx <= tsx) {
							Source.x = tsx - dx;
							Source.w = imgw - Source.x;
							Target.x = tsx;
							Target.w = Source.w;
						}
						if (dy <= tsy && dy + imgh > tey) {
							Source.y = tsy - dy;
							Source.h = th;
							Target.y = tsy;
							Target.h = th;
						} else if (dy >= tsy && (dy + imgh) > tey) {
							Source.h = imgh - ((dy + imgh) - tey);
							Target.h = Source.h;//(dy + imgh) - tey;
							//cout << "ah " << Source.h << "\t" << dy << "\tImgHeight:>" << imgh << "; img-maxy::>" << (dy + imgh) << "; rect-maxy::>" << tey << "=="<<(h+y)<<"\n";
						} else if (dy <= tsy) {
							Source.y = tsy - dy;
							Source.h = imgh - Source.y;
							Target.y = tsy;
							Target.h = Source.h;
						}


						// TQSG_Rect(dx, dy, imgw, imgh);//debug
						if (frame < 0 || frame >= Textures.size()) {
							Paniek("<IMAGE>.Tile(" + to_string(x) + "," + to_string(y) + "," + to_string(w) + "," + to_string(h) + ") Frame(" + to_string(frame) + "/" + to_string(Frames()) + "): Out of frame boundaries (framecount: " + to_string(Textures.size()) + ")"); return;
						}


						Target.x = AltScreen.X(Target.x);
						Target.y = AltScreen.Y(Target.y);
						Target.w = AltScreen.W(Target.w);
						Target.h = AltScreen.H(Target.h);
						SDL_RenderCopy(_Screen->gRenderer, Textures[frame], &Source, &Target);
					}
				}
				//TQSG_ViewPort(ox, oy, ow, oh);
				//TQSG_Color(180, 0, 255);
				//TQSG_Rect(tsx, tsy, tw, th,true);

#else
				auto
					x = ax + _originx,
					y = ay + _originy,
					ix = aix,
					iy = aiy;

				_LastError = "";
				// todo: Fix issues with negative ix
				/*???
				if (iy>0)
					iy = (y + (Height() - iy)) % Height();
				if (ix > 0)
					//ix = (x+ (Width() - ix)) % Width();
					//ix = (x - (Width() + ix)) % Width();
					ix = -(ix % Width());
					//*/
				if (ix < 0) {
					//cout << "neg x:" << ix << " to ";
					ix = (x - (Width() + ix)) % Width();
					//cout << ix << "\n";
				}
				if (iy < 0) {
					//cout << "neg x:" << ix << " to ";
					iy = (y - (Height() + iy)) % Height();
					//cout << ix << "\n";
				}
				//int ox, oy, ow, oh;
				//TQSG_GetViewPort(&ox, &oy, &ow, &oh);
				int tsx, tsy, tex, tey, tw, th;
				int imgh = Height();
				int imgw = Width();
				/*
				tsx = max(ox, x);
				tsy = max(oy, y);
				tex = min(ow + ox, x + w); tw = tex - tsx;
				tey = min(oh + oy, y + h); th = tey - tsy;
				*/
				tsx = x;
				tsy = y;
				tex = w + x;
				tey = h + y;
				tw = w;
				th = h;
				if (tw <= 0 || th <= 0) return; // Nothing to do but getting bugged!
				//cout << "TILE: Rect("<<x<<","<<y<<") "<<w<<"x"<<h<<" "<<"\n";
				//cout << "\tViewPort(" << tsx << "," << tsy << "," << tw << "[" << tex << "]" << "," << th << "[" << tey << "])\n";
				SDL_Rect Target, Source;
				//TQSG_ViewPort(tsx, tsy, tw, th);
				//TQSG_Rect(tsx, tsy, tw, th);
				//cout << "for (int dy = tsy("<<tsy<<") - iy("<<iy<<")(" << (tsy - iy) << "); dy < tey(" << tey << "); dy += imgh(" << imgh << ")) \n";
				SDL_SetTextureColorMod(Textures[frame], _red, _green, _alpha);
				SDL_SetTextureBlendMode(Textures[frame], SDLBlend());
				SDL_SetTextureAlphaMod(Textures[frame], _alpha);
				for (int dy = tsy - iy; dy < tey; dy += imgh) {
					//cout << "(" << x << "," << y << ")\tdy:" << dy << "; tsy:" << tsy << " imgh:" << imgh << " th:" << th << "\n";
					for (int dx = tsx - ix; dx < tex; dx += imgw) {
						//cout << "\t\tDrawTile(" << dx << "," << dy << "," << imgw << "," << imgh << ")\n";
						Target.x = dx;
						Target.y = dy;
						Target.w = imgw;
						Target.h = imgh;
						Source.x = 0;
						Source.y = 0;
						Source.w = imgw;
						Source.h = imgh;
						//cout << "tgt (" << Target.x << "," << Target.y << ") " << Target.w << "x" << Target.h<<"\n";
						//cout << "src (" << Source.x << "," << Source.y << ") " << Source.w << "x" << Source.h<<"; Frame:"<<frame<<"\n\n";
						//cout << "("<<x<<","<<y<<")\tdx:" << dx << "; tsx:" << tsx << " imgw:" << imgw << " tw:" << tw<<"\n";
						if (dx >= tsx && (dx + imgw) > tex) {
							Source.w = imgw - ((dx + imgw) - tex);
							Target.w = Source.w; //(dx + imgw) - tex;
							//cout << "aw " << Source.w << "\n";
						} else if (dx <= tsx) {
							Source.x = tsx - dx;
							Source.w = imgw - Source.x;
							Target.x = tsx;
							Target.w = Source.w;
						}
						if (dy <= tsy && dy + imgh > tey) {
							Source.y = tsy - dy;
							Source.h = th;
							Target.y = tsy;
							Target.h = th;
						} else if (dy >= tsy && (dy + imgh) > tey) {
							Source.h = imgh - ((dy + imgh) - tey);
							Target.h = Source.h;//(dy + imgh) - tey;
							//cout << "ah " << Source.h << "\t" << dy << "\tImgHeight:>" << imgh << "; img-maxy::>" << (dy + imgh) << "; rect-maxy::>" << tey << "=="<<(h+y)<<"\n";
						} else if (dy <= tsy) {
							Source.y = tsy - dy;
							Source.h = imgh - Source.y;
							Target.y = tsy;
							Target.h = Source.h;
						}


						// TQSG_Rect(dx, dy, imgw, imgh);//debug
						if (frame < 0 || frame >= Textures.size()) {
							Paniek("<IMAGE>.Tile(" + to_string(x) + "," + to_string(y) + "," + to_string(w) + "," + to_string(h) + "," + to_string(frame) + "): Out of frame boundaries (framecount: " + to_string(Textures.size()) + ")"); return;
						}
						SDL_RenderCopy(_Screen->gRenderer, Textures[frame], &Source, &Target);
					}
				}
				//TQSG_ViewPort(ox, oy, ow, oh);
				//TQSG_Color(180, 0, 255);
				//TQSG_Rect(tsx, tsy, tw, th,true);
#endif
			} catch (runtime_error re) {
				char t[255];
				sprintf_s(t, "TImage::Tile(%d,%d,%d,%d,%d,%d,%d):", ax, ay, w, h, frame, aix, aiy);
				_LastError = string(t) + string(re.what());
				cout << "ERROR: " << _LastError << endl;
				SetColor(255, 0, 0);
				Rect(aix, aiy, w, h);
			}
		}
#pragma endregion

#pragma region ImageFont
		const int TryFmtMax = 4;
		char TryFmt[TryFmtMax][200]{
			"%d.png",
			"%03d.png",
			"%d.bmp",
			"%03d.bmp"
		};
		char TryFmt2[TryFmtMax][200]{
			"%d.%d.png",
			"%03d.%03d.png",
			"%d.%d.bmp",
			"%03d.03d.bmp"
		};

		class _____TIMAGEFONTCHAR {
		public:
			_____TIMAGEFONT* Parent{ nullptr };
			SDL_Texture* ChImg{nullptr};
			int
				hotx{ 0 },
				hoty{ 0 },
				width{ 0 },
				height{ 0 };
			uint64
				defs{ 0 };
			~_____TIMAGEFONTCHAR() {
				if (ChImg) SDL_DestroyTexture(ChImg);
			}
			_____TIMAGEFONTCHAR(_____TIMAGEFONT* Ouwe, SDL_Texture* _Img, int _x=0, int _y=0, int _w=0, int _h=0) {
				Parent = Ouwe;
				ChImg = _Img;
				hotx = _x;
				hoty = _y;
				width = _w;
				height = _h;
				if (ChImg && (width <= 0 || height <= 0)) {
					SDL_QueryTexture(ChImg, NULL, NULL, &width, &height);
				}
				defs = 1;
				Chat("Character made! ImgPointer(" << (uint64)ChImg << ") hot(" << hotx << "," << hoty << ");  Size: " << width << "x" << height);
			}
			void Draw(int x,int y){
				if (!ChImg) return;
				Chat("Draw char at (" << x << "," << y << ")\n"); // debug only!
				int _w, _h;
				SDL_QueryTexture(ChImg, NULL, NULL, &_w, &_h);
				SDL_Rect Target;
				Target.x = AltScreen.X(x + _originx);
				Target.y = AltScreen.Y(y + _originy);
				Target.w = AltScreen.W(_w);
				Target.h = AltScreen.H(_h);
				SDL_SetTextureBlendMode(ChImg, SDLBlend());
				SDL_SetTextureAlphaMod(ChImg, _alpha);
				SDL_SetTextureColorMod(ChImg, _red, _green, _blue);
				SDL_RenderCopy(_Screen->gRenderer, ChImg, NULL, &Target);
			}
		};

		void _____TIMAGEFONT::KillAll() {
			/*
			for (auto& victim : CharPics) {
				victim.second->defs--;
				if (!victim.second->defs) delete victim.second;
			}
			//*/
		}
		//_____TIMAGEFONTCHAR* _____TIMAGEFONT::GetChar(uint32 c) {
		std::shared_ptr<_____TIMAGEFONTCHAR> _____TIMAGEFONT::GetChar(int c) {
			if (!NeedScreen()) return nullptr;
			//if (!CharPics) { Paniek("Internal error!"); return nullptr; }
			//Chat("Getting char #" << c); // ???
			//if (!CharPics.count(c)) {
			if (!CharPics[c]){
				std::string WantFile{ "" };
				int x{ 0 }, y{ 0 }, w{ 0 }, h{ 0 };
				if (Alt) {
					auto Cat{ TrSPrintF("%04x",c) };
					if (Alt->HasValue(Cat, "LINK")) {
						auto Lch = GetChar(ToInt(Alt->Value(Cat, "LINK")));
						Lch->defs++;
						CharPics[c] = Lch;
						return Lch;
					}
					if (Alt->HasValue(Cat, "Entry")) {
						WantFile = pathprefix+Alt->Value(Cat, "Entry");
					}
					if (Alt->HasValue(Cat, "HOTX")) x = ToInt(Alt->Value(Cat, "HOTX"));
					if (Alt->HasValue(Cat, "HOTY")) y = ToInt(Alt->Value(Cat, "HOTY"));
					if (Alt->HasValue(Cat, "WIDTH")) w = ToInt(Alt->Value(Cat, "WIDTH"));
					if (Alt->HasValue(Cat, "HEIGHT")) w = ToInt(Alt->Value(Cat, "HEIGHT"));
				}
				if (!WantFile.size()) {
					for (byte i = 0; i < TryFmtMax; i++) {
						auto fn{ pathprefix + TrSPrintF(TryFmt[i],c) };
						if (FntRes->EntryExists(fn)) {
							Chat("For character #" << c << ", file " << fn << " has been found!");
							WantFile = fn;
							break;
						} else if (c>255) {
							auto a{ c / 256 }, b{ c % 256 };
							auto fn2{ pathprefix + TrSPrintF(TryFmt2[i],a,b) };
							if (FntRes->EntryExists(fn2)) {
								Chat("For character #" << c << ", file " << fn2 << " has been found!");
								WantFile = fn2;
								break;
							}
						}
					}
				}
				if (!WantFile.size()) {
					std::cout << "WARNING! No suitable character image found for #" << c << ". ("<<pathprefix<<")\n";
					//CharPics[c] = new _____TIMAGEFONTCHAR(this, nullptr);
					CharPics[c] = std::make_shared<_____TIMAGEFONTCHAR>(this, nullptr);
				} else {
					//std::cout << "Loading char: " << WantFile << std::endl; // debug
					auto buf = FntRes->B(WantFile);
					auto rwo = SDL_RWFromMem(buf->Direct(), buf->Size());
					auto tex = IMG_LoadTexture_RW(_Screen->gRenderer, rwo, true);
					//CharPics[c] = new _____TIMAGEFONTCHAR(this, tex, x, y, w, h);
					CharPics[c] = std::make_shared< _____TIMAGEFONTCHAR>(this, tex, x, y, w, h);
				}
				if (spaceavg) {
					uint32 count{ 0 }, total{ 0 };
					for (auto i = 0; i < 256 * 256;i++) {
						if (CharPics[i]) {
							count++;
							total += CharPics[i]->width;
						}
					}
					if (count)	spacewidth = total / count;
				}
			}
			return CharPics[c];
		}
		//_____TIMAGEFONTCHAR* _____TIMAGEFONT::GetChar(byte b1, byte b2) {
		std::shared_ptr<_____TIMAGEFONTCHAR> _____TIMAGEFONT::GetChar(byte b1, byte b2) {
			if (b1 == 100 && b2 == 113) return GetChar(34);
			return GetChar(((uint64)b1 * 256) + (uint64)b2);
		}

		void _____TIMAGEFONT::TW(std::string Text, bool Draw, int& x, int& y) {
			int
				_x = x,
				_y = y,
				_lastmove = 0,
				_lineheight = 0;

			byte
				dchar{ 0 };
			for (size_t pos = 0; pos < Text.size(); ++pos) {
				if (dchar) dchar--; else {
					switch (Text[pos]) {
					case '\0': goto Klaar; // I doubt this will ever be possible, but it's rather a safty precaution.
					case '\1':
					case '\2':
					case '\3':
					case '\4':
					case '\5':
					case '\6':
					case '\7':
						break;
					case '\t':
						do {} while ((++x) % TabWidth != 0);
						break;
					case '\n':
						_y += _lineheight;
						_lineheight = 0;
						_x = x;
						break;
					case '\r':
						_x = x;
						break;
					case '|': {
						if (pos + 1 >= Text.size()) {
							printf("\x1b[31mError\x1b[37m Double char line end error! (%d/%d/%s)\x1b[0m\n", (int)pos, (int)Text.size(), Text.c_str());
							if (IgnoreDoubleCharError)
								break;
							else
								Paniek("Double char line end error!");
						}
						auto ch{ GetChar((byte)Text[pos + 1],(byte)Text[pos + 2]) };
						dchar = 2;
						if (Draw) ch->Draw(_x, _y);
						_x += ch->width;
						_lineheight = std::max(_lineheight, ch->height);
					} break;
					case ' ':
						_x += spacewidth;
						break;
					default: {
						auto ch{ GetChar((int32)Text[pos]) };
						//Chat("Draw (" << ch << ") " << Draw << "!\n"); // debug only
						if (Draw) ch->Draw(_x, _y);
						_x += ch->width;
						_lineheight = std::max(_lineheight, ch->height);
					} break;
					}
				}
			}
			Klaar:
			if (!Draw) { x = _x; y = _y+_lineheight; }
		}
		int _____TIMAGEFONT::Width(std::string Text) {
			int rw{ 0 }, rh{ 0 };
			TW(Text, false, rw, rh);
			return rw;
		}

		_____TIMAGEFONT::_____TIMAGEFONT(JCR6::JT_Dir Res, std::string p) {
			using namespace Units;
			//CharPics.clear();
			//CharPics[0] = std::make_shared <_____TIMAGEFONTCHAR>(this, nullptr); // Let's force something
			for (int i = 0; i < 256 * 256; i++) CharPics[i] = nullptr; // Make sure all instances are null pointers.
			if (p.size()) {
				p = ChReplace(p, '\\', '/');
				if (!Suffixed(p, "/")) p += "/";
			}
			FntRes = Res;
			pathprefix = p;
		}
		int _____TIMAGEFONT::Height(std::string Text) {
			int rw{ 0 }, rh{ 0 };
			TW(Text, false, rw, rh);
			return rh;

		}
		void _____TIMAGEFONT::Text(std::string Text, int x, int y, Align ax,Align ay) {
			int sx{0}, sy{0};
			switch (ax) {
			case Align::Left:
				sx = x; break;
			case Align::Right:
				sx = x- Width(Text);
				break;
			case Align::Center:
				sx = x - (Width(Text) / 2); // -(x / 2);
				break;
			default:
				Paniek("Unknown horizontal alignment"); return;
			}
			switch (ay) {
			case Align::Top:
				sy = y; break;
			case Align::Bottom:
				sy = y - Height(Text);
				break;
			case Align::Center:
				sy = y - (Height(Text) / 2); //- (y / 2);
				break;
			default:
				Paniek("Unknown vertical alignment"); return;
			}
			TW(Text, true, sx, sy);
		}

		void _____TIMAGEFONT::Dark(std::string _Text, int x, int y, Align ax , Align ay) {
			auto __r{ _red }, __g{ _green }, __b{ _blue }, __alpha{ _alpha };
			SetColor(0, 0, 0, 255);
			for (int ix = x - 1; ix <= x + 1; ++ix) for (int iy = y - 1; iy <= y + 1; ++iy) Text(_Text, ix, iy, ax, ay);
			SetColor(__r, __g, __b, 255);
			Text(_Text, x, y, ax, ay);
			SetAlpha(__alpha);
		}
#pragma endregion

}
}
