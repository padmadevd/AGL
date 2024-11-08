#ifndef AGL_INPUT
#define AGL_INPUT

#include <Base/Events/event.hpp>
#include <Base/Events/eventDispatcher.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL.h>

enum KeyCode{

	KEY_RETURN = 0,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    KEY_EXCLAIM,
    KEY_QUOTEDBL,
    KEY_HASH,
    KEY_PERCENT,
    KEY_DOLLAR,
    KEY_AMPERSAND,
    KEY_QUOTE,
    KEY_LEFTPAREN,
    KEY_RIGHTPAREN,
    KEY_ASTERISK,
    KEY_PLUS,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_COLON,
    KEY_SEMICOLON,
    KEY_LESS,
    KEY_EQUALS,
    KEY_GREATER,
    KEY_QUESTION,
    KEY_AT,

    KEY_LEFTBRACKET,
    KEY_BACKSLASH,
    KEY_RIGHTBRACKET,
    KEY_CARET,
    KEY_UNDERSCORE,
    KEY_BACKQUOTE,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,

    KEY_LCTRL,
    KEY_LSHIFT,
    KEY_LALT,
    KEY_RCTRL,
    KEY_RSHIFT,
    KEY_RALT,
};

enum KeyState{

	KEYSTATE_PRESSED = 1,
	KEYSTATE_RELEASED = 2,
	KEYSTATE_UP = 4,
	KEYSTATE_DOWN = 8
};

class EventKeyPress : public Event{

    public:
        EventKeyPress(KeyCode code)
            : m_keyCode(code){
        }

        std::string GetEventType() const override{
            return "AGL_EVENT_KEY_PRESS";
        }

        ~EventKeyPress() override = default;

    public:
        KeyCode m_keyCode;
};

class EventKeyRelease : public Event{

    public:
        EventKeyRelease(KeyCode code)
            : m_keyCode(code){
        }

        std::string GetEventType() const override{
            return "AGL_EVENT_KEY_RELEASE";
        }

        ~EventKeyRelease() override = default;

    public:
        KeyCode m_keyCode;
};

class KeyInput{

    public:

        KeyInput();
        void PollEvents();
        KeyState GetKeyState(KeyCode key);
        bool isKeyPressed(KeyCode key);
        bool isKeyReleased(KeyCode key);
        bool isKeyUp(KeyCode key);
        bool isKeyDown(KeyCode key);
        unsigned int GetSDLKey(KeyCode key);

    private:
        KeyState m_keyState[91];
};

enum TouchType{

	TOUCH_DOWN,
	TOUCH_MOTION
};

class TouchState{

    public:
        SDL_FingerID fingerID;
        TouchType type;
        Uint32 time;
        float x1, y1;
        float x2, y2;
        float dx, dy;
};

enum GestureType{

	GESTURE_TAP,
	// GESTURE_DOUBLETAP,
	GESTURE_HOLD,
	GESTURE_SWIPE,
	GESTURE_DRAG
};

#include <glm/glm.hpp>

class EventGesture : public Event{

    public:

        EventGesture(GestureType type, glm::vec2 start, glm::vec2 end, glm::vec2 dv)
            : m_type(type), m_start(start), m_end(end), m_dv(dv){
        }

        std::string GetEventType() const override{
            return "AGL_EVENT_GESTURE";
        }

        ~EventGesture() override = default;

    public:
        GestureType m_type;
        glm::vec2 m_start, m_end, m_dv;
};

class GestureInput{

    public:

        GestureInput();
        void PollEvents();
    
    private:

        TouchState m_touchState[10];
        Uint32 tapTime;
};

#endif