#include <AGL/agl.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_events.h>

#include<iostream>

EventDispatcher agl_eventDispatcher;
Window *agl_window;
BatchRenderer2D *agl_renderer;
KeyInput *agl_keyInput;
GestureInput *agl_gestureInput;

bool AGLInit(const char *title, int x, int y, int width, int height){

	bool success = true;

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS|SDL_INIT_TIMER|SDL_INIT_SENSOR) != 0){
		success = false;
	}
	if(TTF_Init() != 0){
		success = false;
	}
	if(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG) == 0){
		success = false;
	}
	AGLInitAudio(48000, AUDIO_16B_PER_SAMPLE, 2, 2048, AUDIO_OGG|AUDIO_MP3|AUDIO_WAV);
	
	SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

	agl_window = new Window(title, x, y, width, height);
	agl_renderer = new BatchRenderer2D({width, height}, 10, 4000, 6000, 16);
	agl_keyInput = new KeyInput();
	agl_gestureInput = new GestureInput();

	InitParticleSystem();

	return success;
}

void AGLQuit(){

	delete agl_keyInput;
	delete agl_gestureInput;
	delete agl_renderer;
	delete agl_window;

	QuitParticleSystem();
	AGLQuitAudio();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

void PollEvents(){

	agl_window->PollEvents();
	agl_keyInput->PollEvents();
	agl_gestureInput->PollEvents();
	agl_eventDispatcher.DispatchEvents();
}

Window& GetWindowInstance(){
	return *agl_window;
}

BatchRenderer2D& GetRendererInstance(){
	return *agl_renderer;
}

KeyInput& GetKeyInputInstance(){
	return *agl_keyInput;
}

GestureInput& GetGestureInputInstance(){
	return *agl_gestureInput;
}

EventDispatcher& GetEventDispatcherInstance(){
	return agl_eventDispatcher;
}