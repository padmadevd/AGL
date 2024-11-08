#include <Base/Events/eventDispatcher.hpp>

#include <iostream>

// class EventDispatcher{

//     public:

//         void Register(EventHandler* handler, std::string eventType);
//         void UnRegister(std::string eventType, std::string handlerName);
//         void QueueEvent(const Event* event);
//         void TriggerEvent(const Event* event);
//         void DispatchEvents();

//     private:

//         std::vector<const Event*> m_eventQueue;
//         std::unordered_map<std::string, EventHandler*> m_handlers;
// };

void EventDispatcher::Register(std::string eventType, EventHandler* handler){

    m_handlers[eventType].emplace_back(handler);
}

void EventDispatcher::UnRegister(std::string eventType, std::string handlerName){

    if(m_handlers.find(eventType) == m_handlers.end())
        return;
    
    for(auto i = m_handlers[eventType].begin(); i < m_handlers[eventType].end(); i++){

        if((*i.base())->GetHandlerName() == handlerName){
            delete *i.base();
            m_handlers[eventType].erase(i);
        }
    }    

    if(m_handlers[eventType].size() == 0)
        m_handlers.erase(eventType);

}

void EventDispatcher::QueueEvent(Event* event){

    m_eventQueue.emplace_back(event);
}

void EventDispatcher::TriggerEvent(Event* event){

    if(m_handlers.find(event->GetEventType()) != m_handlers.end()){
        for(auto handler : m_handlers[event->GetEventType()]){
            handler->Call(event);
        }
    }
}

void EventDispatcher::DispatchEvents(){

    for(int e = 0; e < m_eventQueue.size(); e++){

        const Event *event = m_eventQueue[e];
        if(m_handlers.find(event->GetEventType()) != m_handlers.end()){

            std::vector<EventHandler*> &handlers =  m_handlers[m_eventQueue[e]->GetEventType()];
            for(int h = 0; h < handlers.size(); h++){
                handlers[h]->Call(m_eventQueue[e]);
            }
        }
        delete event;
    }
    m_eventQueue.clear();
}