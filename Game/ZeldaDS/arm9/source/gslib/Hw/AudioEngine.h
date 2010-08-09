#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "gslib/Core/Types.h"

namespace PlayMode
{
	enum Type { Loop, Once };
}

namespace AudioEngine
{
	typedef uint32 MusicId;
	typedef uint32 SoundId;

	// Load a single bank at one time (must unload before loading a new one)
	void LoadBank(const char* soundBankFile);
	void UnloadBank();

	void PlayMusic(MusicId musicId, PlayMode::Type playMode = PlayMode::Loop);
	void StopMusic();
	bool IsPlayingMusic();
	void SetMusicVolume(float volumeRatio); // [0,1]

	void PlaySound(SoundId SoundId);
	void StopAllSounds();

	void SetPaused(bool paused);
}

#endif // AUDIO_ENGINE_H
