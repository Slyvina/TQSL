// Lic:
// TQSL/Source/TQSG.cpp
// Tricky's Quick SDL2 Graphics
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


#define TQSG_Debug

#ifdef TQSG_Debug
#define Chat(cht) std::cout << "\x1b[35mTQSG Debug>\t\x1b[0m" << cht <<"\n"
#else
#define Chat(cht)
#endif


#include <TQSG.hpp>
#include <SlyvString.hpp>
#include <SlyvHSVRGB.hpp>
#include <SlyvStream.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace TQSG {


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
			_scaley{ 1.0 };
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
		};
		__AltScreen AltScreen;

		std::string LastError() { return _LastError; }
#pragma region Paniek
		static TQSG_PanicType TQSG_Panic{ nullptr };
		static void DefaultPanic(std::string str) {
			SDL_Window* Win{ nullptr };
			if (_Screen) Win = _Screen->gWindow;
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

		void SetAlpha(byte a) { _alpha = a; _LastError = ""; }
		void SetColor(byte r, byte g, byte b) { _red = r; _green = g; _blue = b; _LastError = ""; }
		void SetColorHSV(double Hue, double Sat, double Value) {
			Units::hsv _hsv{ Hue,Sat,Value };
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

		void Flip(int minticks) {
			if (!NeedScreen()) return;
			//SDL_UpdateWindowSurface(gWindow);
			static auto oud{ SDL_GetTicks() };
			static auto mt{ 26 };
			if (minticks >= 0) mt = minticks;
			while (minticks && (SDL_GetTicks() - oud < mt)) SDL_Delay(1);
			oud = SDL_GetTicks();
			SDL_RenderPresent(_Screen->gRenderer);
		}

		void Line(int start_x, int start_y, int end_x, int end_y) {
			if (!NeedScreen()) return;
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			SDL_RenderDrawLine(_Screen->gRenderer, start_x, start_y, end_x, end_y);
		}

		void Rect(int x, int y, int width, int height, bool open) {
			SDL_Rect r{ x,y,width,height };
			Rect(&r, open);
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

		void ACircle(int center_x, int center_y, int radius, int segments ) {
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
			auto ret{ new _____TIMAGE(J,entry) };
			if (_LastError.size()) { delete ret; return nullptr; }
			return std::shared_ptr<_____TIMAGE>(ret);
		}

		void Plot(int x, int y) {
			_LastError = "";
			if (!NeedScreen()) return;
			SDL_SetRenderDrawColor(_Screen->gRenderer, _red, _green, _blue, _alpha);
			SDL_RenderDrawPoint(_Screen->gRenderer, x, y);
		}

		int32 DesktopWidth() {
			_LastError = "";
			SDL_DisplayMode mode;
			if (!NeedSDL()) return 0;
			if (!SDL_GetDesktopDisplayMode(0, &mode)) _LastError = SDL_GetError();			
			return mode.w;
		}

		int32 DesktopHeight() {
			_LastError = "";
			SDL_DisplayMode mode;
			if (!NeedSDL()) return 0;
			if (!SDL_GetDesktopDisplayMode(0, &mode)) _LastError = SDL_GetError();
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
				char FE[400];
				sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				_LastError = FE;
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
			if (!Frames()) {
				_LastError = "<Image>->Width(): No Frames"; return 0;
				return 0;
			}
			int w, h;
			SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
			return w;
		}

		int _____TIMAGE::Height() {
			_LastError = "";
			if (!Frames()) {
				_LastError = "<Image>->Height(): No Frames"; return 0;
				return 0;
			}
			int w, h;
			SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
			return h;
		}

		uint64 _____TIMAGE::img_cnt{ 0 };
		_____TIMAGE::_____TIMAGE(std::string file) {
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
					char FE[300];
					sprintf_s(FE, 295, "Unable to load image %s!\nSDL_image Error: %s", file.c_str(), IMG_GetError());
					_LastError = FE;
					return;
				}
				//Create texture from surface pixels
				auto newTexture = SDL_CreateTextureFromSurface(_Screen->gRenderer, surf);
				if (newTexture == NULL) {
					char FE[300];
					sprintf_s(FE, 295, "Unable to create texture from %s!\nSDL Error: %s", file.c_str(), SDL_GetError());
					_LastError = FE;
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
				if (!Textures.size()) { _LastError = "No textures loaded!"; }
			}
		}

		_____TIMAGE::~_____TIMAGE() {
			Chat("Image " << _ID << " destroyed");
			KillAllFrames();
		}

		void _____TIMAGE::StretchDraw(int x, int y, int w, int h, int frame) {
			if (!NeedScreen()) return;
			_LastError = "";
			if (frame < 0 || frame >= Textures.size()) {
				char FE[400];
				sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
				_LastError = FE;
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

	}
}