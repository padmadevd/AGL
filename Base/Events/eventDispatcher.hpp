#ifndef EVENT_DISPATCHER
#define EVENT_DISPATCHER

#include <Base/Events/event.hpp>
#include <Base/Events/eventHandler.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class EventDispatcher{

    public:
        void Register(std::string eventType, EventHandler* handler);
        void UnRegister(std::string eventType, std::string handlerName);
        void QueueEvent(Event* event);
        void TriggerEvent(Event* event);
        void DispatchEvents();

    private:

        std::vector<Event*> m_eventQueue;
        std::unordered_map<std::string, std::vector<EventHandler*>> m_handlers;
};

#endif