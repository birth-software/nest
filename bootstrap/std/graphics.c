#include <std/graphics.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

global_variable u8 use_x11 = 0;

fn GraphicsWindow* graphics_window_from_glfw(GLFWwindow* window)
{
    return (GraphicsWindow*)window;
}

fn GLFWwindow* glfw_window_from_graphics(GraphicsWindow* window)
{
    return (GLFWwindow*)window;
}

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

GraphicsWindow* graphics_window_create(GraphicsWindowCreate create)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(create.size.width, create.size.height, string_to_c(create.name), 0, 0);

    return graphics_window_from_glfw(window);
}

u8 graphics_window_should_close(GraphicsWindow* window)
{
    return glfwWindowShouldClose(glfw_window_from_graphics(window));
}

void graphics_poll_events()
{
    glfwPollEvents();
}

GraphicsWindowSize graphics_window_size_get(GraphicsWindow* window)
{
    GLFWwindow* w = glfw_window_from_graphics(window);
    GraphicsWindowSize result;
    glfwGetWindowSize(w, (int*)&result.width, (int*)&result.height);

    return result;
}

#ifdef _WIN32
HANDLE graphics_win32_window_get(GraphicsWindow* window)
{
    return glfwGetWin32Window(glfw_window_from_graphics(window));
}
#endif

#ifdef __linux__
Display* graphics_x11_display_get()
{
    return glfwGetX11Display();
}

Window graphics_x11_window_get(GraphicsWindow* window)
{
    return glfwGetX11Window(glfw_window_from_graphics(window));
}
#endif
