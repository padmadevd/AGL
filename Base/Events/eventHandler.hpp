#ifndef AGL_EVENT_HANDLER
#define AGL_EVENT_HANDLER

#include <Base/Events/event.hpp>
#include <functional>

class EventHandler{

    using Callback = std::function<void(const Event*, void*)>;

    public:

        EventHandler(Callback callback, void *userData, std::string name)
            : m_callback(callback), m_userData(userData), m_name(name){
        }

        std::string GetHandlerName(){
            return m_name;
        }

        void Call(const Event *event){
            m_callback(event, m_userData);
        }

        void operator()(const Event *event){
            m_callback(event, m_userData);
        }

    private:

        const std::string m_name;
        Callback m_callback;
        void *m_userData;
};

#endif