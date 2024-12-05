#pragma once

#include <std/base.h>
#include <std/graphics.h>
#include <std/os.h>

STRUCT(UI_MousePosition)
{
    f64 x;
    f64 y;
};

STRUCT(UI_State)
{
    Arena* arena;
    UI_MousePosition mouse_position;
    OSEventMouseButtonEvent mouse_button_events[OS_EVENT_MOUSE_BUTTON_COUNT];
    u8 focused:1;
};

STRUCT(UI_WidgetFlags)
{
    u32 draw_text:1;
    u32 reserved:31;
};

STRUCT(UI_Widget)
{
    UI_WidgetFlags flags;
    String string;
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

EXPORT void ui_state_select(UI_State* state);
EXPORT u8 ui_build_begin(OSWindow window, f64 frame_time, OSEventQueue* event_queue);
EXPORT void ui_build_end();
EXPORT void ui_draw();
