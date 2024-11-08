#ifndef AGL_AUDIO_H
#define AGL_AUDIO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cstdint>

enum AudioFileFormat{

	AUDIO_MP3 = MIX_INIT_MP3,
	AUDIO_OGG = MIX_INIT_OGG,
	AUDIO_FLAC = MIX_INIT_FLAC,
	AUDIO_WAV = MIX_INIT_WAVPACK,
	AUDIO_MIDI = MIX_INIT_MID,
	AUDIO_MOD = MIX_INIT_MOD,
	AUDIO_OPUS = MIX_INIT_OPUS
};

enum AudioSampleSize{

	AUDIO_16B_PER_SAMPLE = AUDIO_S16SYS,
	AUDIO_32B_PER_SAMPLE = AUDIO_S32SYS
};

void AGLInitAudio(int outputFreq, uint64_t sampleSize, int channels, int chunkSize, uint64_t fileFormatMask);
void AllocateMixChannels(int count);

struct Sound;

Sound* LoadSoundFromFile(const char *path);
void SetSoundVolume(Sound *sound, int vol);
void FreeSound(Sound *sound);

void PlayChannel(int channel, Sound *sound, int loops);
void PlayChannelTimed(int channel, Sound *sound, int loops, int ms);
void PauseChannel(int channel);
void ResumeChannel(int channel);
void HaltChannel(int channel);

void SetChannelVolume(int channel, int vol);

struct Music;

Music* LoadMusicFromFile(const char *path);
void SetMusicVolume(Music *music, int vol);
void SetMusicChannelVolume(int vol);
void PlayMusic(Music *music, int loops);
void FadeInMusic(Music *music, int loops, int ms);
void PauseMusic();
void ResumeMusic();
void HaltMusic();
void FadeOutMusic(int ms);
void FreeMusic(Music *music);

void SetMasterVolume(int vol);

// struct SFX;

// SFX* EffectPitchShift(int pitch);
// void EffectSetPitch(SFX *effect, int pitch);

// SFX* EffectAmplify(int amplitude);
// void EffectSetAmplitude(SFX *effect, int amplitude);

// SFX* EffectReverb(int size);
// void EffectSetReverbSize(SFX *effect, int size);

// SFX* EffectPanner(int left, int right);
// void EffectSetPanning(SFX *effect, int left, int right);

// void FreeEffect(SFX *effect);

// void AddEffect(int channel, SFX *effect);
// void RemoveEffect(int channel, SFX *effect);
// void RemoveAllEffects(int channel);

void AGLQuitAudio();

#endif