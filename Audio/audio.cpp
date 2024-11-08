#include <Audio/audio.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

struct Sound{

	Mix_Chunk *sdlSound;
};

struct Music{
	Mix_Music *sdlMusic;
	int vol;
};

int musicChannelVol;

void AGLInitAudio(int outputFreq, uint64_t sampleSize, int channels, int chunkSize, uint64_t fileFormatMask){

	SDL_Init(SDL_INIT_AUDIO);
	Mix_Init(fileFormatMask);
	Mix_OpenAudio(outputFreq, sampleSize, channels, chunkSize);

	musicChannelVol = 100;
}

void AllocateMixChannels(int count){
	Mix_AllocateChannels(count);
}

Sound* LoadSoundFromFile(const char *path){
	Sound *sound = new Sound;
	sound->sdlSound = Mix_LoadWAV(path);
	return sound;
}
void SetSoundVolume(Sound *sound, int vol){
	Mix_VolumeChunk(sound->sdlSound, (vol/100.f)*MIX_MAX_VOLUME);
}
void FreeSound(Sound *sound){
	Mix_FreeChunk(sound->sdlSound);
	delete sound;
}

void PlayChannel(int channel, Sound *sound, int loops){
	Mix_PlayChannel(channel, sound->sdlSound, loops);
}
void PlayChannelTimed(int channel, Sound *sound, int loops, int ms){
	Mix_PlayChannelTimed(channel, sound->sdlSound, loops, ms);
}
void PauseChannel(int channel){
	Mix_Pause(channel);
}
void ResumeChannel(int channel){
	Mix_Resume(channel);
}
void HaltChannel(int channel){
	Mix_HaltChannel(channel);
}
void SetChannelVolume(int channel, int vol){
	Mix_Volume(channel, (vol/100.f)*MIX_MAX_VOLUME);
}

Music* LoadMusicFromFile(const char *path){
	Music *music = new Music;
	music->sdlMusic = Mix_LoadMUS(path);
	music->vol = 100;
	return music;
}
void SetMusicVolume(Music *music, int vol){
	music->vol = vol;
}
void SetMusicChannelVolume(int vol){
	musicChannelVol = vol;
}
void PlayMusic(Music *music, int loops){
	Mix_VolumeMusic((music->vol/100.f)*(musicChannelVol/100.f)*MIX_MAX_VOLUME);
	Mix_PlayMusic(music->sdlMusic, loops);
}
void FadeInMusic(Music *music, int loops, int ms){
	Mix_VolumeMusic((music->vol/100.f)*(musicChannelVol/100.f)*MIX_MAX_VOLUME);
	Mix_FadeInMusic(music->sdlMusic, loops, ms);
}
void PauseMusic(){
	Mix_PauseMusic();
}
void ResumeMusic(){
	Mix_ResumeMusic();
}
void HaltMusic(){
	Mix_HaltMusic();
}
void FadeOutMusic(int ms){
	Mix_FadeOutMusic(ms);
}
void FreeMusic(Music *music){
	Mix_FreeMusic(music->sdlMusic);
	delete music;
}

void SetMasterVolume(int vol){
	Mix_MasterVolume((vol/100.f)*MIX_MAX_VOLUME);
}

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

void AGLQuitAudio(){

	Mix_CloseAudio();
	Mix_Quit();
}