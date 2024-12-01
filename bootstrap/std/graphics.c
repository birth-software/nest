#include <std/graphics.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

global_variable u8 use_x11 = 0;

void os_graphics_init(u8 should_use_x11)
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

global_variable OSWindowResize* resize_callback;
global_variable OSWindowRefresh* refresh_callback;

fn void os_framebuffer_size_callback(GLFWwindow* w, int width, int height)
{
    void* context = glfwGetWindowUserPointer(w);
    if (resize_callback)
    {
        resize_callback(w, context, width, height);
    }
}

fn void os_window_refresh_callback(GLFWwindow* w)
{
    void* context = glfwGetWindowUserPointer(w);
    if (refresh_callback)
    {
        refresh_callback(w, context);
    }
}

OSWindow os_window_create(OSWindowCreate create)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(create.size.width, create.size.height, string_to_c(create.name), 0, 0);
    glfwSetWindowUserPointer(window, create.context);
    glfwSetFramebufferSizeCallback(window, &os_framebuffer_size_callback);
    glfwSetWindowRefreshCallback(window, &os_window_refresh_callback);
    resize_callback = create.resize_callback;
    refresh_callback = create.refresh_callback;

    return window;
}

u8 os_window_should_close(OSWindow window)
{
    return glfwWindowShouldClose(window);
}

void os_poll_events()
{
    glfwPollEvents();
}

OSWindowSize os_window_size_get(OSWindow window)
{
    OSWindowSize result;
    glfwGetWindowSize(window, (int*)&result.width, (int*)&result.height);

    return result;
}

OSCursorPosition os_window_cursor_position_get(OSWindow window)
{
    OSCursorPosition result;
    glfwGetCursorPos(window, &result.x, &result.y);
    return result;
}

#ifdef _WIN32
HANDLE win32_window_get(OSWindow window)
{
    return glfwGetWin32Window(window);
}
#endif

#ifdef __linux__
Display* x11_display_get()
{
    return glfwGetX11Display();
}

Window x11_window_get(OSWindow window)
{
    return glfwGetX11Window(window);
}
#endif
