#include <std/ui_core.h>
#include <std/format.h>
#include <std/string.h>

global_variable UI_State* ui_state = 0;

void ui_state_select(UI_State* state)
{
    ui_state = state;
}

UI_State* ui_state_get()
{
    return ui_state;
}

fn Arena* ui_build_arena()
{
    auto* arena = ui_state->build_arenas[ui_state->build_count % array_length(ui_state->build_arenas)];
    return arena;
}

fn UI_Key ui_key_null()
{
    UI_Key key = {};
    return key;
}

UI_State* ui_state_allocate(Renderer* renderer, RenderWindow* window)
{
    Arena* arena = arena_init(GB(8), MB(2), MB(2));
    UI_State* state = arena_allocate(arena, UI_State, 1);
    state->renderer = renderer;
    state->render_window = window;
    state->arena = arena;
    state->widget_table.length = 4096;
    state->widget_table.pointer = arena_allocate(arena, UI_WidgetSlot, state->widget_table.length);
    
    for (u64 i = 0; i < array_length(state->build_arenas); i += 1)
    {
        state->build_arenas[i] = arena_init(GB(8), MB(2), MB(2));
    }

    return state;
}

auto text_end_delimiter = strlit("##");
auto hash_start_delimiter = strlit("###");

fn String ui_text_from_key_string(String string)
{
    String result = string;
    auto index = string_first_ocurrence(string, text_end_delimiter);
    if (index < string.length)
    {
        result.length = index;
    }
    return result;
}

fn String ui_hash_from_key_string(String string)
{
    String result = string;
    auto index = string_first_ocurrence(string, hash_start_delimiter);
    if (index < string.length)
    {
        result = s_get_slice(u8, string, index, string.length);
    }

    return result;
}

fn UI_Key ui_key_from_string(UI_Key seed, String string)
{
    UI_Key key = ui_key_null();

    if (string.length)
    {
        key = seed;

        for (u64 i = 0; i < string.length; i += 1)
        {
            key.value = ((key.value << 5) + key.value) + string.pointer[i];
        }
    }

    return key;
}

fn UI_Key ui_key_from_string_format(UI_Key seed, char* format, ...)
{
    u8 buffer[256];
    va_list args;
    va_start(args, format);
    auto string = format_string_va((String)array_to_slice(buffer), format, args);
    va_end(args);
    auto result = ui_key_from_string(seed, string);
    return result;
}

fn u8 ui_key_equal(UI_Key a, UI_Key b)
{
    return a.value == b.value;
}

UI_Widget* ui_widget_from_key(UI_Key key)
{
    UI_Widget* result = 0;

    if (!ui_key_equal(key, ui_key_null()))
    {
        auto index = key.value & (ui_state->widget_table.length - 1);
        for (UI_Widget* widget = ui_state->widget_table.pointer[index].first; widget; widget = widget->hash_next)
        {
            if (ui_key_equal(widget->key, key))
            {
                result = widget;
                break;
            }
        }
    }

    return result;
}

UI_Widget* ui_widget_make_from_key(UI_WidgetFlags flags, UI_Rect rect, UI_Key key)
{
    auto* widget = arena_allocate(ui_state->arena, UI_Widget, 1);
    *widget = (UI_Widget)
    {
        .flags = flags,
        .rect = rect,
        .key = key,
        .background_color = Color4(0, 0, 0, 1),
        // TODO: modify
        .parent = ui_state->root,
    };

    auto* parent = widget->parent;
    if (parent)
    {
        auto* ptr = &parent->last;
        auto* previous_last = *ptr;
        if (previous_last)
        {
            previous_last->next = widget;
        }
        *ptr = widget;

        if (!parent->first)
        {
            parent->first = widget;
        }
        parent->last = widget;
    }
    
    return widget;
}

UI_Widget* ui_widget_make(UI_WidgetFlags flags, UI_Rect rect, String string)
{
    // TODO:
    auto seed = ui_key_null();

    auto hash_string = ui_hash_from_key_string(string);
    auto key = ui_key_from_string(seed, hash_string);

    auto* widget = ui_widget_make_from_key(flags, rect, key);

    if (flags.draw_text)
    {
        widget->text_string = ui_text_from_key_string(string);
    }

    return widget;
}

UI_Widget* ui_widget_make_format(UI_WidgetFlags flags, UI_Rect rect, const char* format, ...)
{
    va_list args;
    u8 buffer[4096];
    va_start(args, format);
    auto string = format_string_va((String)array_to_slice(buffer), format, args);
    va_end(args);

    auto* result = ui_widget_make(flags, rect, string);
    return result;
}

UI_Signal ui_signal_from_widget(UI_Widget* widget)
{
    UI_Rect rect = widget->rect;
    UI_Signal signal = {
        .clicked_left = 
            (widget->flags.clickable & (ui_state->mouse_button_events[OS_EVENT_MOUSE_LEFT].action == OS_EVENT_MOUSE_RELEASE)) &
            ((ui_state->mouse_position.x >= rect.x0) & (ui_state->mouse_position.x <= rect.x1)) &
            ((ui_state->mouse_position.y >= rect.y0) & (ui_state->mouse_position.y <= rect.y1)),
    };
    return signal;
}

u8 ui_build_begin(OSWindow os_window, f64 frame_time, OSEventQueue* event_queue)
{
    ui_state->build_count += 1;
    auto* build_arena = ui_build_arena();
    arena_reset(build_arena);
    ui_state->frame_time = frame_time;
    ui_state->os_window = os_window;
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
                auto previous_button_event = ui_state->mouse_button_events[button.button];
                switch (button.event.action)
                {
                    case OS_EVENT_MOUSE_RELAX:
                        unreachable();
                    case OS_EVENT_MOUSE_RELEASE:
                        {
                            assert(previous_button_event.action == OS_EVENT_MOUSE_PRESS);
                        } break;
                    case OS_EVENT_MOUSE_PRESS:
                        {
                            assert(previous_button_event.action == OS_EVENT_MOUSE_RELAX);
                        } break;
                    case OS_EVENT_MOUSE_REPEAT:
                        {
                            unreachable();
                        } break;
                }

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

    for (u64 i = 0; i < ui_state->widget_table.length; i += 1)
    {
        for (UI_Widget* widget = ui_state->widget_table.pointer[i].first, *next = 0; widget; widget = next)
        {
            next = widget->hash_next;

            if (ui_key_equal(widget->key, ui_key_null()) || widget->last_build_touched + 1 < ui_state->build_count)
            {
                trap();
            }
        }
    }

    auto framebuffer_size = os_window_framebuffer_size_get(os_window);

    ui_state->root = ui_widget_make_format(
        (UI_WidgetFlags) {},
        (UI_Rect) {
            .x0 = 0,
            .y0 = 0,
            .x1 = framebuffer_size.width,
            .y1 = framebuffer_size.height,
        },
        "window_root_{u64}", os_window
    );

    return open;
}

void ui_build_end()
{
    // Clear release button presses
    for (u32 i = 0; i < array_length(ui_state->mouse_button_events); i += 1)
    {
        auto* event = &ui_state->mouse_button_events[i];
        if (event->action == OS_EVENT_MOUSE_RELEASE)
        {
            event->action = OS_EVENT_MOUSE_RELAX;
        }
    }
}

fn RenderRect render_rect(UI_Rect rect)
{
    return (RenderRect) {
        .x0 = rect.x0,
        .y0 = rect.y0,
        .x1 = rect.x1,
        .y1 = rect.y1,
    };
}

void ui_draw()
{
    UI_Widget* root = ui_state->root;

    UI_Widget* widget = root;
    RenderWindow* window = ui_state->render_window;
    Renderer* renderer = ui_state->renderer;

    while (1)
    {
        if (widget->flags.draw_background)
        {
            window_render_rect(window, (RectDraw) {
                .color = widget->background_color,
                .vertex = render_rect(widget->rect),
            });
        }

        if (widget->flags.draw_text)
        {
            window_render_text(renderer, window, widget->text_string, Color4(1, 1, 1, 1), RENDER_FONT_TYPE_PROPORTIONAL, widget->rect.x0, widget->rect.y0);
        }

        if (widget->first)
        {
            widget = widget->first;
        }
        else if (widget->next)
        {
            widget = widget->next;
        }
        else if (widget->parent == ui_state->root)
        {
            break;
        }
        else if (widget->parent)
        {
            widget = widget->parent;
        }
        else
        {
            unreachable();
        }
    }
}

