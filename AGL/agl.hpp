#ifndef AGL
#define AGL

#include <Base/gl.hpp>
#include <Base/window.hpp>
#include <Base/input.hpp>
#include <Graphics/graphics.hpp>
#include <Graphics/aglShader.hpp>
#include <BatchRenderer/batchRenderer.hpp>
#include <Audio/audio.hpp>

#include <ParticleSystem/particles.hpp>

#include <Physics/polygon2d.hpp>

bool AGLInit(const char *title, int x, int y, int width, int height);
void AGLQuit();

void PollEvents();

Window& GetWindowInstance();
BatchRenderer2D& GetRendererInstance();
KeyInput& GetKeyInputInstance();
GestureInput& GetGestureInputInstance();
EventDispatcher& GetEventDispatcherInstance();

#endif