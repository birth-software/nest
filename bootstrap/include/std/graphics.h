#pragma once

#include <std/base.h>
#include <std/os.h>

STRUCT(GraphicsWindowSize)
{
    u32 width;
    u32 height;
};

STRUCT(GraphicsWindowCreate)
{
    String name;
    GraphicsWindowSize size;
};

STRUCT(GraphicsCursorPosition)
{
    f64 x;
    f64 y;
};

typedef struct GraphicsWindow GraphicsWindow;

EXPORT void graphics_init(u8 should_use_x11);
EXPORT GraphicsWindow* graphics_window_create(GraphicsWindowCreate create);
EXPORT u8 graphics_window_should_close(GraphicsWindow* window);
EXPORT void graphics_poll_events();
EXPORT GraphicsWindowSize graphics_window_size_get(GraphicsWindow* window);
EXPORT void graphics_window_consume_resize(GraphicsWindow* window);
EXPORT GraphicsCursorPosition graphics_window_cursor_position_get(GraphicsWindow* window);

#ifdef __linux__
typedef unsigned long XID;
typedef struct _XDisplay Display;
typedef XID Window;

EXPORT Display* graphics_x11_display_get();
EXPORT Window graphics_x11_window_get(GraphicsWindow* window);
#endif

#ifdef _WIN32
EXPORT HANDLE graphics_win32_window_get(GraphicsWindow* window);
#endif
