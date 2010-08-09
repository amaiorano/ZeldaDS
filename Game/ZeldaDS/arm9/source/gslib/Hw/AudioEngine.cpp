#include "AudioEngine.h"
#include <maxmod9.h>
#include <vector>
#include <string.h>
#include "gslib/Core/Core.h"

namespace AudioEngine
{
	// Questions about maxmod:
	// - Are you supposed to have only one soundbank with all the game music + sound, and just load/unload the ones you need?
	// - When loading with mmInitDefault(char*), does it fully load data into memory with mmLoad or does it stream?

	namespace
	{
		// Constants
		const uint16 MaxMusicElems = 3;
		const uint16 MaxSoundElems = 10;
		const MusicId InvalidMusicId = static_cast<MusicId>(-1);
		const SoundId InvalidSoundId = static_cast<SoundId>(-1);
		const mm_sfxhand InvalidSoundHandle = static_cast<mm_sfxhand>(-1);

		struct MusicData
		{
			MusicData() 
				: mLoaded(false)
			{
			}

			void Reset()
			{
				mLoaded = false;
			}

			bool mLoaded;
		};

		// Types
		struct SoundData
		{
			SoundData() 
				: mLoaded(false)
			{
			}

			void Reset()
			{
				mLoaded = false;
				ResetParams(InvalidSoundId);
			}

			void ResetParams(SoundId SoundId)
			{
				memset(&mParams, 0, sizeof(mParams));
				mParams.id = SoundId;
				mParams.rate = (1<<10); // 1
				mParams.volume = 255; // Max
				mParams.panning = 128; // Center
			}

			bool IsPlaying() const
			{
				//@NOTE: We can't actually check if the sound has stopped apparently
				return mLoaded && mParams.handle != 0;
			}

			bool mLoaded;
			mm_sound_effect mParams;
		};

		// Vars
		const char* gActiveBank = 0;
		std::vector<MusicData> gMusicDataList(MaxMusicElems);
		std::vector<SoundData> gSoundDataList(MaxSoundElems);
		MusicId gPlayingMusicId = InvalidMusicId;

		// Functions
		void LazyLoadMusic(MusicId id)
		{
			MusicData& data = gMusicDataList.at(id);

			if (!data.mLoaded)
			{
				mmLoad(id);
				data.mLoaded = true;
			}
		}
		void UnloadMusic(MusicId id)
		{
			MusicData& data = gMusicDataList.at(id);
			if (data.mLoaded)
			{
				mmUnload(id);
				data.mLoaded = false;
			}
		}

		void LazyLoadSound(SoundId id)
		{
			SoundData& data = gSoundDataList.at(id);
			if (!data.mLoaded)
			{
				mmLoadEffect(id);
				data.mLoaded = true;
			}
		}
		void UnloadSound(SoundId id)
		{
			SoundData& data = gSoundDataList.at(id);
			if (data.mLoaded)
			{
				mmUnloadEffect(id);
				data.mLoaded = false;
			}
		}
	} // anonymous namespace

	void LoadBank(const char* soundBankFile)
	{
		ASSERT_MSG(gActiveBank == 0, "Need to UnloadBank() first");
		gActiveBank = soundBankFile;
		mmInitDefault(const_cast<char*>(gActiveBank));

		for (size_t i = 0; i < gMusicDataList.size(); ++i)
		{
			gMusicDataList[i].Reset();
		}

		for (size_t i = 0; i < gSoundDataList.size(); ++i)
		{
			gSoundDataList[i].Reset();
		}
	}

	void UnloadBank()
	{
		StopMusic();
		StopAllSounds();
		gActiveBank = 0; // No function to explicitly unload the active bank file (maybe mmInitDefault(NULL)?)
	}

	void PlayMusic(MusicId musicId, PlayMode::Type playMode)
	{
		LazyLoadMusic(musicId);

		//@TODO: Fade current music out, next one in (optional input?)
		mmStart(musicId, playMode == PlayMode::Loop? MM_PLAY_LOOP : MM_PLAY_ONCE);
		gPlayingMusicId = musicId;
	}

	void StopMusic()
	{
		if (gPlayingMusicId != InvalidMusicId)
		{
			// mmStop() doesn't always work - sometimes, trying to play the music again
			// after stopping causes it not to play! This workaround seems to do the trick.
			//mmStop();
			mmPosition(0);
			mmPause();

			UnloadMusic(gPlayingMusicId);
			gPlayingMusicId = InvalidMusicId;
		}
	}

	bool IsPlayingMusic()
	{
		return mmActive();
	}

	void SetMusicVolume(float volumeRatio)
	{
		const mm_word volume = static_cast<mm_word>(volumeRatio * 1024);
		mmSetModuleVolume(volume);
	}

	void PlaySound(SoundId SoundId)
	{
		LazyLoadSound(SoundId);
		SoundData& data = gSoundDataList[SoundId];
		data.ResetParams(SoundId); // Reset to default values
		data.mParams.handle = mmEffectEx(&data.mParams); // Strangely, mmSoundEx doesn't store handle into input
	}

	void StopAllSounds()
	{
		mmEffectCancelAll();
		for (size_t i = 0; i < gSoundDataList.size(); ++i)
		{
			UnloadSound(i);
			gSoundDataList[i].Reset(); // Not absolutely necessary, but cleaner
		}
	}

	void SetPaused(bool paused)
	{
		if (gPlayingMusicId != InvalidMusicId)
		{
			paused? mmPause() : mmResume();
		}

		// For effects, we set the playback rate to 0 on pause, or it's previous value on resume
		for (size_t i = 0; i < gSoundDataList.size(); ++i)
		{
			SoundData& data = gSoundDataList[i];
			if (data.IsPlaying())
			{
				mmEffectRate(data.mParams.handle, paused? 0 : data.mParams.rate);
			}
		}
	}

} // namespace AudioEngine

