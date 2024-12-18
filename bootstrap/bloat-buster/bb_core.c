#if BB_CI == 0
#include <bloat-buster/bb_core.h>

#include <std/virtual_buffer.h>
#include <std/window.h>
#include <std/render.h>
#include <std/shader_compilation.h>
#include <std/image_loader.h>
#include <std/font_provider.h>
#include <std/ui_core.h>
#include <std/ui_builder.h>

#define default_font_height (64)
auto proportional_font_height = default_font_height;
auto monospace_font_height = default_font_height;

fn TextureIndex white_texture_create(Arena* arena, Renderer* renderer)
{
    u32 white_texture_width = 1024;
    u32 white_texture_height = white_texture_width;
    auto* white_texture_buffer = arena_allocate(arena, u32, white_texture_width * white_texture_height);
    memset(white_texture_buffer, 0xff, white_texture_width * white_texture_height * sizeof(u32));

    auto white_texture = renderer_texture_create(renderer, (TextureMemory) {
        .pointer = white_texture_buffer,
        .width = white_texture_width,
        .height = white_texture_height,
        .depth = 1,
        .format = TEXTURE_FORMAT_R8G8B8A8_SRGB,
    });

    return white_texture;
}

STRUCT(BBPanel)
{
    BBPanel* first;
    BBPanel* last;
    BBPanel* next;
    BBPanel* previous;
    BBPanel* parent;
    f32 parent_percentage;
    Axis2 split_axis;
};

STRUCT(BBWindow)
{
    OSWindow os;
    RenderWindow* render;
    BBWindow* previous;
    BBWindow* next;
    BBPanel* root_panel;
    UI_State* ui;
};

STRUCT(BBGUIState)
{
    Arena* arena;
    Timestamp last_frame_timestamp;
    BBWindow* first_window;
    BBWindow* last_window;
    Renderer* renderer;
    // TODO: should this not be thread local?
    OSEventQueue event_queue;
};
global_variable BBGUIState state;

fn void app_update()
{
    auto frame_end = os_timestamp();
    os_poll_events(&state.event_queue);
    auto frame_ms = os_resolve_timestamps(state.last_frame_timestamp, frame_end, TIME_UNIT_MILLISECONDS);
    state.last_frame_timestamp = frame_end;

    Renderer* renderer = state.renderer;

    BBWindow* window = state.first_window;
    while (likely(window))
    {
        auto* previous = window->previous;
        auto* next = window->next;

        auto* render_window = window->render;
        renderer_window_frame_begin(renderer, render_window);

        ui_state_select(window->ui);

        if (likely(ui_build_begin(window->os, frame_ms, &state.event_queue)))
        {
            ui_font_size(default_font_height);
            ui_pref_width(ui_em(10, 1));
            ui_pref_height(ui_em(2, 1));

            if (unlikely(ui_button(strlit("Hello world\n")).clicked_left))
            {
                print("Clicked on hello world\n");
            }

            if (unlikely(ui_button(strlit("Bye world\n")).clicked_left))
            {
                print("Clicked on bye world\n");
            }

            ui_build_end();

            ui_draw();

            renderer_window_frame_end(renderer, render_window);
        }
        else
        {
            if (previous)
            {
                previous->next = next;
            }

            if (next)
            {
                next->previous = previous;
            }

            if (state.first_window == window)
            {
                state.first_window = next;
            }

            if (state.last_window == window)
            {
                state.last_window = previous;
            }
        }

        window = next;
    }
}

fn void window_refresh_callback(OSWindow window, void* context)
{
    unused(window);
    unused(context);
    app_update();
}

void run_app()
{
    state.arena = arena_init(MB(512), MB(2), MB(2));

    os_graphics_init((OSGraphicsInitializationOptions) {
        .should_use_x11 = 1,
    });
    state.renderer = renderer_initialize(state.arena);

    state.first_window = state.last_window = arena_allocate(state.arena, BBWindow, 1);
    state.first_window->os = os_window_create((OSWindowCreate) {
        .name = strlit("Bloat Buster"),
        .size = {
            .width = 1024,
            .height= 768,
        },
        .refresh_callback = &window_refresh_callback,
    });

    if (!state.first_window->os)
    {
        failed_execution();
    }

    state.first_window->render = renderer_window_initialize(state.renderer, state.first_window->os);
    state.first_window->ui = ui_state_allocate(state.renderer, state.first_window->render);
    state.first_window->root_panel = arena_allocate(state.arena, BBPanel, 1);
    state.first_window->root_panel->parent_percentage = 1.0f;
    state.first_window->root_panel->split_axis = AXIS2_X;

    auto font_path = 
#ifdef _WIN32
        strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf");
#elif defined(__linux__)
        strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf");
#elif defined(__APPLE__)
strlit("/Users/david/Library/Fonts/FiraSans-Regular.ttf");
#else
        strlit("WRONG_PATH");
#endif

    window_rect_texture_update_begin(state.first_window->render);

    auto white_texture = white_texture_create(state.arena, state.renderer);
    auto monospace_font = font_texture_atlas_create(state.arena, state.renderer, (TextureAtlasCreate) {
        .font_path = font_path,
        .text_height = monospace_font_height,
    });
    auto proportional_font = monospace_font;

    window_queue_rect_texture_update(state.first_window->render, RECT_TEXTURE_SLOT_WHITE, white_texture);
    renderer_queue_font_update(state.renderer, state.first_window->render, RENDER_FONT_TYPE_MONOSPACE, monospace_font);
    renderer_queue_font_update(state.renderer, state.first_window->render, RENDER_FONT_TYPE_PROPORTIONAL, proportional_font);

    window_rect_texture_update_end(state.renderer, state.first_window->render);

    state.last_frame_timestamp = os_timestamp();

    while (state.first_window)
    {
        app_update();
    }

    // TODO: deinitialization
}

#endif
