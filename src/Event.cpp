#include "Event.hpp"

void EventDispatcher::register_handler(EventType type, EventHandler handler)
{
    m_handlers[type].push_back(handler);
}

void EventDispatcher::dispatch(Event const& event)
{
    if (m_handlers.contains(event.type)) {
        for (auto const& handler : m_handlers[event.type]) {
            handler(event);
        }
    }
}
