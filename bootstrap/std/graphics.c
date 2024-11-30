#include <std/graphics.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

global_variable u8 use_x11 = 0;

STRUCT(GraphicsWindow)
{
    GLFWwindow* handle;
    u8 resized:1;
    u32 width;
    u32 height;
};
#define MAX_WINDOW_COUNT (32)
global_variable GraphicsWindow windows[MAX_WINDOW_COUNT];
global_variable u32 window_count = 0;

void graphics_init(u8 should_use_x11)
{
#ifdef __linux__
    use_x11 = should_use_x11;
    int platform_hint = use_x11 ? GLFW_PLATFORM_X11 : GLFW_PLATFORM_WAYLAND;
    glfwInitHint(GLFW_PLATFORM, platform_hint);
#endif

    if (glfwInit() != GLFW_TRUE)
    {
        failed_execution();
    }
}

fn void framebuffer_size_callback(GLFWwindow* w, int width, int height)
{
    GraphicsWindow* window = glfwGetWindowUserPointer(w);
    assert(window->handle == w);
    window->width = width;
    window->height = height;
    window->resized = 1;
}

typedef void (* GLFWframebuffersizefun)(GLFWwindow* window, int width, int height);

GraphicsWindow* graphics_window_create(GraphicsWindowCreate create)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GraphicsWindow* window = &windows[window_count];
    *window = (GraphicsWindow) {
        .handle = glfwCreateWindow(create.size.width, create.size.height, string_to_c(create.name), 0, 0),
    };
    glfwSetWindowUserPointer(window->handle, window);
    glfwSetFramebufferSizeCallback(window->handle, &framebuffer_size_callback);
    window->width = create.size.width;
    window->height = create.size.height;

    return window;
}

void graphics_window_consume_resize(GraphicsWindow* window)
{
    assert(window->resized);
    window->resized = 0;
}

u8 graphics_window_should_close(GraphicsWindow* window)
{
    return glfwWindowShouldClose(window->handle);
}

void graphics_poll_events()
{
    glfwPollEvents();
}

GraphicsWindowSize graphics_window_size_get(GraphicsWindow* window)
{
    GraphicsWindowSize result;
    glfwGetWindowSize(window->handle, (int*)&result.width, (int*)&result.height);

    return result;
}

GraphicsCursorPosition graphics_window_cursor_position_get(GraphicsWindow* window)
{
    GraphicsCursorPosition result;
    glfwGetCursorPos(window->handle, &result.x, &result.y);
    return result;
}

#ifdef _WIN32
HANDLE graphics_win32_window_get(GraphicsWindow* window)
{
    return glfwGetWin32Window(window->handle);
}
#endif

#ifdef __linux__
Display* graphics_x11_display_get()
{
    return glfwGetX11Display();
}

Window graphics_x11_window_get(GraphicsWindow* window)
{
    return glfwGetX11Window(window->handle);
}
#endif
