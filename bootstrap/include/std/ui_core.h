#pragma once

#include <std/base.h>
#include <std/graphics.h>
#include <std/os.h>
#include <std/render.h>

STRUCT(UI_Key)
{
    u64 value;
};

STRUCT(UI_MousePosition)
{
    f64 x;
    f64 y;
};

STRUCT(UI_WidgetFlags)
{
    u32 draw_text:1;
    u32 draw_background:1;
    u32 clickable:1;
    u32 reserved:30;
};

UNION(UI_Rect)
{
    struct
    {
        u32 x0;
        u32 y0;
        u32 x1;
        u32 y1;
    };
};

STRUCT(UI_Widget)
{
    UI_Widget* hash_previous;
    UI_Widget* hash_next;

    UI_Widget* first;
    UI_Widget* last;
    UI_Widget* next;
    UI_Widget* previous;
    UI_Widget* parent;

    UI_Key key;
    u64 last_build_touched;

    UI_WidgetFlags flags;
    String text_string;
    UI_Rect rect;
    Color background_color;
};

STRUCT(UI_WidgetSlot)
{
    UI_Widget* first;
    UI_Widget* last;
};
declare_slice(UI_WidgetSlot);

STRUCT(UI_State)
{
    Arena* arena;
    Arena* build_arenas[2];
    Renderer* renderer;
    RenderWindow* render_window;
    OSWindow os_window;
    u64 build_count;
    f64 frame_time;
    UI_Widget* root;
    UI_MousePosition mouse_position;
    Slice(UI_WidgetSlot) widget_table;
    OSEventMouseButtonEvent mouse_button_events[OS_EVENT_MOUSE_BUTTON_COUNT];
    u8 focused:1;
};

enum
{
    UI_SignalFlag_ClickedLeft = (1 << 0),
};

typedef u32 UI_SignalFlags;

STRUCT(UI_Signal)
{
    UI_Widget* widget;
    union
    {
        UI_SignalFlags flags;
        struct
        {
            u32 clicked_left:1;
            u32 reserved:31;
        };
    };
};

EXPORT UI_State* ui_state_allocate(Renderer* renderer, RenderWindow* window);
EXPORT void ui_state_select(UI_State* state);
EXPORT u8 ui_build_begin(OSWindow window, f64 frame_time, OSEventQueue* event_queue);
EXPORT void ui_build_end();
EXPORT void ui_draw();
EXPORT UI_Signal ui_signal_from_widget(UI_Widget* widget);
EXPORT UI_State* ui_state_get();

EXPORT UI_Widget* ui_widget_make(UI_WidgetFlags flags, UI_Rect rect, String string);
