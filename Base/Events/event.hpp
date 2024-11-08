#ifndef AGL_EVENT
#define AGL_EVENT

#include <string>

class Event{

    public:
    
    virtual std::string GetEventType() const = 0;
    virtual ~Event() = default;
};

#endif