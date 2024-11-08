#include <Base/window.hpp>
#include <Base/Events/event.hpp>
#include <Base/Events/eventDispatcher.hpp>
#include <Base/gl.hpp>

#include <SDL2/SDL.h>
#include "SDL2/SDL_events.h"

#include <cstdio>
#include <vector>

extern EventDispatcher agl_eventDispatcher;

void Window::OpenGLESWindow(const char *title, int x, int y, int width, int height){

	m_windowSDL = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
	if(!m_windowSDL){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "window creation failed\n");
		return;
	}
	m_windowID = SDL_GetWindowID(m_windowSDL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GLContext glctx = SDL_GL_CreateContext(m_windowSDL);
	if(!glctx){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "opengl context failed");
		return;
	}
	SDL_GL_MakeCurrent(m_windowSDL, glctx);

	SDL_GL_GetDrawableSize(m_windowSDL, &m_width_px, &m_height_px);
}

void Window::OpenGLWindow(const char *title, int x, int y, int width, int height){

	m_windowSDL = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
	
	if(!m_windowSDL){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "window creation failed\n");
		return;
	}
	m_windowID = SDL_GetWindowID(m_windowSDL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GLContext glctx = SDL_GL_CreateContext(m_windowSDL);
	if(!glctx){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "opengl context creation failed");
		return;
	}
	SDL_GL_MakeCurrent(m_windowSDL, glctx);

	SDL_GL_GetDrawableSize(m_windowSDL, &m_width_px, &m_height_px);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		printf("error loading glad\n");
		fflush(stdout);
		return;
	}
}

Window::Window(const char *title, int x, int y, int width, int height)
 	: m_width(width), m_height(height), m_shouldClose(false){

	#ifdef __ANDROID__
		OpenGLESWindow(title, x, y, width, height);
	#else
		OpenGLWindow(title, x, y, width, height);
	#endif
 }

void Window::PollEvents(){

	SDL_Event event;
	std::vector<SDL_Event> unprocessedEvents;
	while(SDL_PollEvent(&event) > 0){
		
		if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == m_windowID){
			SDL_GetWindowSize(m_windowSDL, &m_width, &m_height);
			SDL_GetWindowSizeInPixels(m_windowSDL, &m_width_px, &m_height_px);
			agl_eventDispatcher.QueueEvent(new EventWindowResize(m_width, m_height, m_width_px, m_height_px, this));
		}
		else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == m_windowID){
			m_shouldClose = true;
		}
		else if(event.type == SDL_QUIT){
			m_shouldClose = true;
			unprocessedEvents.push_back(event);
		}
		else{
			unprocessedEvents.push_back(event);
		}
	}

	SDL_PeepEvents(unprocessedEvents.data(), unprocessedEvents.size(), SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

bool Window::ShouldClose(){
	return m_shouldClose;
}

void Window::SwapBuffers(){
	SDL_GL_SwapWindow(m_windowSDL);
}

int Window::GetWidth(){
	return m_width;
}

int Window::GetHeight(){
	return m_height;
}

int Window::GetWidthInPixels(){
	return m_width_px;
}

int Window::GetHeightInPixels(){
	return m_height_px;
}

void Window::SetFullScreen(bool fullscreen){

	if(fullscreen)
		SDL_SetWindowFullscreen(m_windowSDL, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(m_windowSDL, 0);
}

void Window::SetResizable(bool resizable){

	if(resizable)
		SDL_SetWindowResizable(m_windowSDL, SDL_TRUE);
	else
		SDL_SetWindowResizable(m_windowSDL, SDL_FALSE);
}
        
void Window::Minimize(){
	SDL_MinimizeWindow(m_windowSDL);
}

void Window::Maximize(){
	SDL_MaximizeWindow(m_windowSDL);
}

void Window::Restore(){
	SDL_RestoreWindow(m_windowSDL);
}

Window::~Window(){
	SDL_DestroyWindow(m_windowSDL);
}