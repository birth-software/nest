#include <std/ui_core.h>
#include <std/format.h>

global_variable UI_State* ui_state = 0;

void ui_state_select(UI_State* state)
{
    ui_state = state;
}

u8 ui_build_begin(OSWindow window, f64 frame_time, OSEventQueue* event_queue)
{
    u8 open = 1;
    for (u32 generic_event_index = 0; generic_event_index < event_queue->descriptors.length; generic_event_index += 1)
    {
        auto event_descriptor = event_queue->descriptors.pointer[generic_event_index];
        u32 event_index = event_descriptor.index;

        switch (event_descriptor.type)
        {
        case OS_EVENT_TYPE_MOUSE_BUTTON:
            {
                auto button = event_queue->mouse_buttons.pointer[event_index];
                ui_state->mouse_button_events[button.button] = button.event;
            } break;
        case OS_EVENT_TYPE_WINDOW_FOCUS:
            {
            } break;
        case OS_EVENT_TYPE_CURSOR_POSITION:
            {
                auto mouse_position = event_queue->cursor_positions.pointer[event_index];
                ui_state->mouse_position = (UI_MousePosition) {
                    .x = mouse_position.x,
                    .y = mouse_position.y,
                };
            } break;
        case OS_EVENT_TYPE_CURSOR_ENTER:
            {
                todo();
            } break;
        case OS_EVENT_TYPE_WINDOW_POSITION:
            {
                // event_queue->window_positions.pointer[event_index];
                // todo();
            } break;
        case OS_EVENT_TYPE_WINDOW_CLOSE:
            {
                open = 0;
            } break;
        }
    }

    auto framebuffer_size = os_window_framebuffer_size_get(window);

    return open;
}

void ui_build_end()
{
}

void ui_draw()
{
}

UI_Widget* ui_widget_make(UI_WidgetFlags flags, String string)
{
    auto* widget = arena_allocate(ui_state->arena, UI_Widget, 1);
    *widget = (UI_Widget)
    {
        .string = string,
        .flags = flags,
    };
    return widget;
}

UI_Widget* ui_widget_make_format(UI_WidgetFlags flags, const char* format, ...)
{
    va_list args;
    u8 buffer[4096];
    va_start(args, format);
    auto string = format_string_va((String)array_to_slice(buffer), format, args);
    va_end(args);

    auto* result = ui_widget_make(flags, string);
    return result;
}
