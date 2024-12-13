#pragma once
#include <functional>
#include <unordered_map>

// Enum for different event types
enum class EventType {
    KEY_PRESS,
    KEY_RELEASE,
    MOUSE_MOVE,
    MOUSE_BUTTON_PRESS,
    MOUSE_BUTTON_RELEASE,
    MOUSE_SCROLL,
    WINDOW_RESIZE,
    WINDOW_CLOSE,
};

struct Event {
    EventType type;
    float time_delta;
    // prepare structs for different kind of inputs
    union {
        struct {
            int key_code;
        } key_event;
        struct {
            int x, y;
        } mouse_move_event;
        struct {
            int button, x, y;
        } mouse_button_event;
        struct {
            double xoffset, yoffset;
        } mouse_scroll_event;
    };
};

// Event dispatcher class
class EventDispatcher {
public:
    using EventHandler = std::function<void(Event const&)>;

    void register_handler(EventType type, EventHandler handler);
    void dispatch(Event const& event);

private:
    std::unordered_map<EventType, std::vector<EventHandler>> m_handlers;
};