#pragma once

#include <std/base.h>
#include <std/os.h>

typedef void* OSWindow;

typedef void OSWindowResize(OSWindow window, void* context, u32 width, u32 height);
typedef void OSWindowRefresh(OSWindow window, void* context);

STRUCT(OSWindowSize)
{
    u32 width;
    u32 height;
};

STRUCT(OSWindowCreate)
{
    String name;
    OSWindowSize size;
    void* context;
    OSWindowResize* resize_callback;
    OSWindowRefresh* refresh_callback;
};

STRUCT(OSCursorPosition)
{
    f64 x;
    f64 y;
};

EXPORT void os_graphics_init(u8 should_use_x11);
EXPORT OSWindow os_window_create(OSWindowCreate create);
EXPORT u8 os_window_should_close(OSWindow window);
EXPORT void os_poll_events();
EXPORT OSWindowSize os_window_size_get(OSWindow window);
EXPORT OSCursorPosition os_window_cursor_position_get(OSWindow window);

#ifdef __linux__
typedef unsigned long XID;
typedef struct _XDisplay Display;
typedef XID Window;

EXPORT Display* x11_display_get();
EXPORT Window x11_window_get(OSWindow window);
#endif

#ifdef _WIN32
EXPORT HANDLE win32_window_get(GraphicsWindow* window);
#endif
