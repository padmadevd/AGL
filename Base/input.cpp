#include <Base/input.hpp>

#include <iostream>

// class KeyInput{

//     public:

//         KeyInput();
//         KeyState GetKeyState(KeyCode key);
//         bool isKeyPressed(KeyCode key);
//         bool isKeyReleased(KeyCode key);
//         bool isKeyUp(KeyCode key);
//         bool isKeyDown(KeyCode key);
//         unsigned int GetSDLKey(KeyCode key);

//     private:

//         int SDLOnKeyUp(void *userData, SDL_Event *event);
//         int SDLOnKeyDown(void *userData, SDL_Event *event);

//     private:
//         KeyState keyState[91];
// };

extern EventDispatcher agl_eventDispatcher;

unsigned int KeyCodeSDL[91] = {

	SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_SPACE,
    SDLK_EXCLAIM,
    SDLK_QUOTEDBL,
    SDLK_HASH,
    SDLK_PERCENT,
    SDLK_DOLLAR,
    SDLK_AMPERSAND,
    SDLK_QUOTE,
    SDLK_LEFTPAREN,
    SDLK_RIGHTPAREN,
    SDLK_ASTERISK,
    SDLK_PLUS,
    SDLK_COMMA,
    SDLK_MINUS,
    SDLK_PERIOD,
    SDLK_SLASH,
   	SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_COLON,
    SDLK_SEMICOLON,
    SDLK_LESS,
    SDLK_EQUALS,
    SDLK_GREATER,
    SDLK_QUESTION,
    SDLK_AT,

    SDLK_LEFTBRACKET,
    SDLK_BACKSLASH,
    SDLK_RIGHTBRACKET,
    SDLK_CARET,
    SDLK_UNDERSCORE,
    SDLK_BACKQUOTE,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_w,
    SDLK_x,
    SDLK_y,
    SDLK_z,

    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
   	SDLK_F10,
   	SDLK_F11,
   	SDLK_F12,

   	SDLK_RIGHT,
   	SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,

    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT
};

KeyInput::KeyInput(){

    for(int i = 0; i < 91; i++)
        m_keyState[i] = KEYSTATE_UP;
}

void KeyInput::PollEvents(){

    for(int i = 0; i < 91; i++){
        if(m_keyState[i]&KEYSTATE_DOWN)
            m_keyState[i] = KEYSTATE_DOWN;
        if(m_keyState[i]&KEYSTATE_UP)
            m_keyState[i] = KEYSTATE_UP;
    }

    SDL_Event event;
    std::vector<SDL_Event> unprocessedEvents;
    while(SDL_PollEvent(&event) > 0){

        if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
            for(int i = 0; i < 91; i++){
                if(KeyCodeSDL[i] == event.key.keysym.sym){
                    agl_eventDispatcher.QueueEvent(new EventKeyPress(KeyCode(i)));
                    m_keyState[i] = KeyState(KEYSTATE_DOWN | KEYSTATE_PRESSED);
                    break;
                }
            }
        }
        else if(event.type == SDL_KEYUP){
            for(int i = 0; i < 91; i++){
                if(KeyCodeSDL[i] == event.key.keysym.sym){
                    agl_eventDispatcher.QueueEvent(new EventKeyRelease(KeyCode(i)));
                    m_keyState[i] = KeyState(KEYSTATE_UP | KEYSTATE_RELEASED);
                    break;
                }
            }
        }
        else{
            unprocessedEvents.push_back(event);
        }
    }

    SDL_PeepEvents(unprocessedEvents.data(), unprocessedEvents.size(), SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

KeyState KeyInput::GetKeyState(KeyCode key){
    return m_keyState[key];
}
bool KeyInput::isKeyPressed(KeyCode key){
    return m_keyState[key] & KEYSTATE_PRESSED;
}
bool KeyInput::isKeyReleased(KeyCode key){
    return m_keyState[key] & KEYSTATE_RELEASED;
}
bool KeyInput::isKeyUp(KeyCode key){
    return m_keyState[key] & KEYSTATE_UP;
}
bool KeyInput::isKeyDown(KeyCode key){
    return m_keyState[key] & KEYSTATE_DOWN;
}
unsigned int KeyInput::GetSDLKey(KeyCode key){
    return KeyCodeSDL[key];
}

GestureInput::GestureInput()
    : tapTime(1000){
    for(int i = 0; i < 10; i++){
        m_touchState[i].fingerID = -1;
    }
}

void GestureInput::PollEvents(){

    SDL_Event event;
    std::vector<SDL_Event> unprocessedEvents;
    while(SDL_PollEvent(&event) > 0){

        if(event.type == SDL_FINGERDOWN){
            
            for(int i = 0; i < 10; i++){

                if(m_touchState[i].fingerID == -1){

                    m_touchState[i].fingerID = event.tfinger.fingerId;
                    m_touchState[i].type = TOUCH_DOWN;
                    m_touchState[i].time = SDL_GetTicks();
                    m_touchState[i].x1 = event.tfinger.x;
                    m_touchState[i].y1 = event.tfinger.y;
                    m_touchState[i].x2 = event.tfinger.x;
                    m_touchState[i].y2 = event.tfinger.y;
                    break;
                }
            }
        }
        else if(event.type == SDL_FINGERMOTION){

            for(int i = 0; i < 10; i++){

                if(m_touchState[i].fingerID == event.tfinger.fingerId){

                    m_touchState[i].type = TOUCH_MOTION;
                    m_touchState[i].x2 = event.tfinger.x;
                    m_touchState[i].y2 = event.tfinger.y;
                    m_touchState[i].dx = event.tfinger.dx;
                    m_touchState[i].dy = event.tfinger.dy;
                    
                    // std::cout<<"drag\n";
                    agl_eventDispatcher.QueueEvent(new EventGesture(GESTURE_DRAG, {m_touchState[i].x1, m_touchState[i].y1}, {m_touchState[i].x2, m_touchState[i].y2}, {m_touchState[i].dx, m_touchState[i].dy}));

                    break;
                }
            }
        }
        else if(event.type == SDL_FINGERUP){

            for(int i = 0; i < 10; i++){

                if(m_touchState[i].fingerID == event.tfinger.fingerId){

                    m_touchState[i].fingerID = -1;

                    if((SDL_GetTicks()-m_touchState[i].time) > tapTime)
                        break;

                    GestureType type;
                    if(m_touchState[i].type == TOUCH_DOWN){
                        type = GESTURE_TAP;
                    }else{
                        type = GESTURE_SWIPE;
                    }
                    agl_eventDispatcher.QueueEvent(new EventGesture(type, {m_touchState[i].x1, m_touchState[i].y1}, {m_touchState[i].x2, m_touchState[i].y2}, {m_touchState[i].dx, m_touchState[i].dy}));
                    break;
                }
            }
        }
        else{
            unprocessedEvents.push_back(event);
        }
    }

    for(int i  = 0; i < 10; i++){
        if(m_touchState[i].fingerID != -1){
            if(m_touchState[i].type == TOUCH_DOWN)
                agl_eventDispatcher.QueueEvent(new EventGesture(GESTURE_HOLD, {m_touchState[i].x1, m_touchState[i].y1}, {m_touchState[i].x2, m_touchState[i].y2}, {m_touchState[i].dx, m_touchState[i].dy}));
        }
    }

    SDL_PeepEvents(unprocessedEvents.data(), unprocessedEvents.size(), SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}