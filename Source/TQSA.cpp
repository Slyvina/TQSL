
#define TQSA_DEBUG

#include <TQSA.hpp>

#ifdef TQSA_DEBUG
#define Chat(abc) std::cout << "\x1b[33mTQSA DEBUG>\t\x1b[0m"<<abc<<"\n"
#else
#define Chat(abc)
#endif

namespace Slyvina {
	namespace TQSA {
		static std::string _LastError{ "" };

		std::string AudioError() { return _LastError; }

		class ME_Init {
		public:
			static std::unique_ptr<ME_Init> MEI;
			bool Initiated{ true };
			ME_Init() { Chat("Audio init, detected!"); }
			~ME_Init() {
				if (Initiated) { Mix_CloseAudio(); Chat("Close audio"); }
				Initiated = false;
			}
		};
		std::unique_ptr<ME_Init> ME_Init::MEI{ nullptr };


		bool Init_TQSA(int demandflags) {
			//if (!ME_Init::MEI) Init_TQSA();
			bool success = true;
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
				std::cout << "\aMix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048) failed\n";
				return false;
			}
			int flags = Mix_Init(MIX_INIT_OGG | MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MID);
			success = success && (flags != 0);
			if (demandflags) success = success && ((flags & demandflags) == demandflags);
			if (!success) std::cout << "Initizing the mixer has failed\n";
			ME_Init::MEI = std::make_unique<ME_Init>();
			return success;
		}

		TAudio LoadAudio(std::string File) {
			_LastError = "";
			auto TA{ new _____TAudio(File.c_str()) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::shared_ptr<_____TAudio>(TA);
		}

		TUAudio LoadUAudio(std::string File) {
			_LastError = "";
			auto TA{ new _____TAudio(File.c_str()) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::unique_ptr<_____TAudio>(TA);
		}

		TAudio LoadAudio(JCR6::JT_Dir JCRResource, std::string Entry) {
			_LastError = "";
			if (!JCRResource->EntryExists(Entry)) { _LastError = "JCR6 resource does not have an entry named: " + Entry; }
			auto TA{ new _____TAudio(JCRResource,Entry) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::shared_ptr<_____TAudio>(TA);
		}

		TUAudio LoadUAudio(JCR6::JT_Dir JCRResource, std::string Entry) {
			return TUAudio();
			_LastError = "";
			if (!JCRResource->EntryExists(Entry)) { _LastError = "JCR6 resource does not have an entry named: " + Entry; }
			auto TA{ new _____TAudio(JCRResource,Entry) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::unique_ptr<_____TAudio>(TA);
		}

		TAudio LoadAudio(std::string JCRResource, std::string Entry) {
			_LastError = "";
			auto J = JCR6::JCR6_Dir(JCRResource);
			if (JCR6::Last()->Error) { _LastError = JCR6::Last()->ErrorMessage; return nullptr; }
			auto TA{ new _____TAudio(J,Entry) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::shared_ptr<_____TAudio>(TA);
		}

		TUAudio LoadUAudio(std::string JCRResource, std::string Entry) {
			_LastError = "";
			auto J = JCR6::JCR6_Dir(JCRResource);
			if (JCR6::Last()->Error) { _LastError = JCR6::Last()->ErrorMessage; return nullptr; }
			auto TA{ new _____TAudio(J,Entry) };
			if (_LastError.size()) {
				delete TA;
				return nullptr;
			}
			return std::unique_ptr<_____TAudio>(TA);
		}

		uint64 _____TAudio::count_audio{ 0 };
		_____TAudio::_____TAudio(const char* File) {
			if (!ME_Init::MEI) Init_TQSA();
			ActualChunk = Mix_LoadWAV(File);
			if (!ActualChunk) std::cout << "Loading from file \"" << File << "\" failed!\n";
		}

		_____TAudio::_____TAudio(JCR6::JT_Dir JCRResource, std::string JCREntry) {
			if (!ME_Init::MEI) Init_TQSA();
			_LastError = "";
			if (!JCRResource->EntryExists(JCREntry)) {
				_LastError = "Loading JCR Entry \"" + JCREntry + "\" not possible as the entry does not exist!\n";
				return;
			}
			SDL_RWops* RWBuf = NULL;
			auto E = JCRResource->Entry(JCREntry);
			//jcr6::JT_EntryReader buf;
			//JCRResource.B(JCREntry, buf);
			//RWBuf = SDL_RWFromMem(buf.pointme(), buf.getsize());
			auto buf = JCRResource->B(JCREntry);
			if (!buf) { _LastError = "Loading JCR6 entry failed!\n" + JCR6::Last()->ErrorMessage; return; }
			RWBuf = SDL_RWFromMem(buf->Direct(), buf->Size());
			ActualChunk = Mix_LoadWAV_RW(RWBuf, 1);
			//if (!JCRResource.EntryExists(JCREntry)) { std::cout << "Loading JCR Entry \"" << JCREntry << "\" failed!\n"; return; }
		}


		_____TAudio::~_____TAudio() {
			if (ActualChunk) Mix_FreeChunk(ActualChunk);
		}

		int _____TAudio::Play(int loops) {
			return Mix_PlayChannel(-1, ActualChunk, loops);
		}

		void _____TAudio::ChPlay(int channel, int loops) {
			Mix_PlayChannel(channel, ActualChunk, loops);
		}

	}
}
