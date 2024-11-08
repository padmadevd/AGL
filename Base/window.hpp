#ifndef AGL_WINDOW
#define AGL_WINDOW

#include <Base/Events/event.hpp>
#include <Base/Events/eventDispatcher.hpp>

#include <SDL2/SDL.h>

class Window;

class EventWindowResize : public Event{

    public:

        EventWindowResize(int width, int height, int width_px, int height_px, Window *window)
            : m_width(width), m_height(height), m_width_px(width_px), m_height_px(height_px), m_window(window){
        }

        std::string GetEventType() const override{
            return "AGL_EVENT_WINDOW_RESIZE";
        }

        ~EventWindowResize() override = default;
        
    public:

        int m_width, m_height;
        int m_width_px, m_height_px;
        Window *m_window;
};

class Window{

    public:

        Window(const char *title, int x, int y, int width, int height);

        bool ShouldClose();
        void PollEvents();
        void SwapBuffers();

        int GetWidth();
        int GetHeight();
        int GetWidthInPixels();
        int GetHeightInPixels();

        void SetFullScreen(bool fullscreen);
        void SetResizable(bool resizable);
        void Minimize();
        void Maximize();
        void Restore(); 

        ~Window();

    private:

        SDL_Window *m_windowSDL;
        int m_windowID;

        int m_width, m_height;
        int m_width_px, m_height_px;

        bool m_shouldClose;

        void OpenGLESWindow(const char *title, int x, int y, int width, int height);
        void OpenGLWindow(const char *title, int x, int y, int width, int height);
};

#endif